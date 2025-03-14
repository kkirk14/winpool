
/**
 * Winpool.submit.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Winpool::submit
 * 
 * Adds a task to the pool's task queue. Once this is called, the pool may
 * start executing the task immediately.
 * 
 * func: Function to execute.
 * arg: Argument to pass to func.
 * 
 * Return Value: Returns a borrowed pointer to a heap-allocated Future which 
 *               can be used to obtain the task's result in the future.
 *               Ownership of this memory is held by the pool until 
 *               completion.
 */
Future *Winpool::submit(WinpoolTask func, void *arg) {
    
    Worker *myWorker = (Worker *)TlsGetValue(this->workerTlsIdx);

    if (myWorker == nullptr) {
        return this->externalSubmit(func, arg);
    }
    else {
        return this->workerSubmit(func, arg, myWorker);
    }
}
