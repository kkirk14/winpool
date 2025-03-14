
/**
 * Future.workerGet.cxx
 */



#include <memory>
#include <windows.h>
#include <assert.h>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Future::workerGet
 * 
 * Helper function for Future::get - only called by worker threads.
 * If this Future is QUEUED, just executes it in the calling thread.
 * If this Future is RUNNING, helps the thread executing it until it 
 * is done.
 * If this Future is DONE, just returns the result.
 * 
 * newOwner: Ownership of this Future will be passed here if it's not 
 *           nullptr.
 * bMyWorker: Borrowed pointer to the Worker instance for the calling 
 *            thread.
 * 
 * Return Value: Returns the result returned by this Future's task.
 */
void *Future::workerGet(UniquePtr<Future> *newOwner, Worker *bMyWorker) {

    UniquePtr<Future> uThis;
    void *res;

    EnterCriticalSection(this->lock);

    // The task hasn't started, execute it yourself
    if (this->status == QUEUED) {
        this->status = RUNNING;
        this->executor = bMyWorker;
        uThis = this->popFromList();
        LeaveCriticalSection(this->lock);

        res = this->func(this->arg);
        
        EnterCriticalSection(this->lock);
        this->res = res;
        this->status = DONE;
        //this->owner->completedList.insertTail(std::move(uThis));
        if (newOwner != nullptr) 
            *newOwner = std::move(uThis);
        WakeAllConditionVariable(&this->condCompleted);
        LeaveCriticalSection(this->lock);
        return res;
    }

    // The task is running, help the worker that is executing it.
    while (this->status == RUNNING) {

        // The executor should never be the worker of the calling thread 
        assert(this->executor != bMyWorker);

        EnterCriticalSection(&this->executor->lock);

        // Executor has task available: steal and execute it
        if (!this->executor->taskQueue.empty()) {
            UniquePtr<Future> helpFut = this->executor->taskQueue.popTail();
            assert(helpFut != nullptr);
            Future *bHelpFut = helpFut.get();
            helpFut->status = RUNNING;
            helpFut->executor = bMyWorker;
            LeaveCriticalSection(&this->executor->lock);
            LeaveCriticalSection(this->lock);

            res = helpFut->func(helpFut->arg);

            EnterCriticalSection(helpFut->lock);
            helpFut->res = res;
            helpFut->status = DONE;
            helpFut->owner->completedList.insertTail(std::move(helpFut));
            WakeAllConditionVariable(&bHelpFut->condCompleted);
            LeaveCriticalSection(bHelpFut->lock);
        }

        // Executor doesn't have any subtasks: yield and check later
        else {
            LeaveCriticalSection(&this->executor->lock);
            LeaveCriticalSection(this->lock);
            Sleep(0); // This yields this thread's time slice
        }
        
        EnterCriticalSection(this->lock);
    }

    // At this point, we know that our task has completed and is on a 
    // completedList
    res = this->res;
    uThis = this->popFromList();
    LeaveCriticalSection(this->lock);
    if (newOwner != nullptr)
        *newOwner = std::move(uThis);
    return res;
}
