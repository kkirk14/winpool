
/**
 * Winpool.workerSubmit.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Winpool::workerSubmit
 * 
 * Helper function for submit() that is called when submit is invoked by a 
 * worker thread.
 * Places the task on the worker's queue instead of on the pool's.
 * 
 * func: Functionto execute.
 * arg: Argument to pass to func.
 * worker: Points to the Worker object with info about the invoking worker
 *         thread.
 * 
 * Return Value: Returns a borrowed pointer to a Future that can be used
 *               to get the task's result in the future.
 */
Future *Winpool::workerSubmit(WinpoolTask func, void *arg, Worker *worker) {
    
    UniquePtr<Future> uFuture = UniquePtr<Future>(
        new Future(func, arg, this, worker)
    );
    Future *bFuture = uFuture.get();

    EnterCriticalSection(bFuture->lock);
    worker->taskQueue.insertHead(std::move(uFuture));
    LeaveCriticalSection(bFuture->lock);

    return bFuture;
}
