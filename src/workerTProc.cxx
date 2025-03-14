
/**
 * workerTProc.cxx
 */



#include <cstdio>
#include <windows.h>
#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * workerTProc
 * 
 * Base function run by the worker thread processes.
 * Loops, searching for tasks in the pool's queue and the other workers' 
 * queues and executing them until the pool shuts down.
 * 
 * arg: Borrowed pointer to a WorkerTProcData instance that contains data about
 *      the pool and about the running thread.
 * 
 * Return Value: Doesn't matter
 */
DWORD WINAPI WinpoolNS::workerTProc(void *arg) {

    BOOL boolRc;

    WorkerTProcData *workerData = (WorkerTProcData *)arg;
    Worker *myWorker = workerData->myWorker;
    DWORD tlsMyWorkerIndex = workerData->tlsMyWorkerIdx;
    Winpool *pool = workerData->pool;

    Worker *workers = pool->workers.get();
    int nWorkers = pool->nWorkers;

    // Set the my worker Tls variable
    boolRc = TlsSetValue(tlsMyWorkerIndex, myWorker);
    if (!boolRc) {
        EnterCriticalSection(pool->lock);
        std::fprintf(stderr, "workerTProc TlsSetValue failed\n");
        std::fflush(stderr);
        LeaveCriticalSection(pool->lock);
        return -1;
    }

    EnterCriticalSection(pool->lock);
    while (pool->running) {

        UniquePtr<Future> futToExec = nullptr;

        // Check pool queue for tasks
        if (!pool->futures.taskQueue.empty()) {
            futToExec = pool->futures.taskQueue.popHead();
            futToExec->status = RUNNING;
            futToExec->executor = myWorker;
        }
        LeaveCriticalSection(pool->lock);

        // Check all worker queues for tasks
        for (int iWorker = 0; 
             iWorker < nWorkers && futToExec == nullptr; 
             iWorker++) {

            Worker *currWorker = &workers[iWorker];
            EnterCriticalSection(&currWorker->lock);
            if (!currWorker->taskQueue.empty()) {
                futToExec = currWorker->taskQueue.popTail();
                futToExec->status = RUNNING;
                futToExec->executor = myWorker;
            }
            LeaveCriticalSection(&currWorker->lock);
        }

        // We found a future - execute it
        if (futToExec != nullptr) {
            
            // Execute the task
            void *futRes = futToExec->func(futToExec->arg);

            Future *bFutToExec = futToExec.get();
            
            // Save the result, add futToExec to its completed list and wake 
            // up any threads waiting for the result
            EnterCriticalSection(futToExec->lock);
            futToExec->status = DONE;
            futToExec->res = futRes;
            futToExec->owner->completedList.insertTail(std::move(futToExec));
            WakeAllConditionVariable(&bFutToExec->condCompleted);
            LeaveCriticalSection(bFutToExec->lock);
        }

        // Sleep for a little before checking again
        else {
            Sleep(1); // 1 ms
        }

        // pool->lock must be held here
        EnterCriticalSection(pool->lock);
    }

    LeaveCriticalSection(pool->lock);

    return 0;
}
