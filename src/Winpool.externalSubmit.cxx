
/**
 * Winpool.externalSubmit.cxx
 */



#include <memory>
#include <windows.h>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Winpool::externalSubmit
 * 
 * Helper function for submit() that is called when submit is invoked by an
 * external thread.
 * Places the task on the pool's queue.
 * 
 * func: Function to execute.
 * arg: Argument to pass to func.
 * 
 * Return Value: Returns a borrowed pointer to a Future that can be used
 *               to get the task's result in the future.
 */
Future *Winpool::externalSubmit(WinpoolTask func, void *arg) {
    
    UniquePtr<Future> uFuture = UniquePtr<Future>(
        new Future(func, arg, this, &this->futures)
    );
    Future *bFuture = uFuture.get();

    EnterCriticalSection(this->lock);
    this->futures.taskQueue.insertTail(std::move(uFuture));
    LeaveCriticalSection(this->lock);

    return bFuture;
}
