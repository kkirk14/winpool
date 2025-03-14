
/**
 * SequentialWinpool.cxx
 * 
 * Simple single-threaded implementation of the Winpool library. This will act
 * as a control against which the actual Winpool library will be tested 
 * against. How much speedup does the multi-threading provide?
 */



#include <memory>
#include <winpool.hxx>



using namespace WinpoolNS;



/**
 * Future constructor
 * 
 * func: Task to execute.
 * arg: Argument to pass to func.
 * bPool: Borrowed pointer to the pool.
 * owner: Borrowed pointer to the FutureOwner that protects this Future.
 */
Future::Future(WinpoolTask func, 
               void *arg, 
               Winpool *bPool, 
               FutureOwner *owner) {
    
    this->func = func;
    this->arg = arg;
    this->bPool = bPool;
    this->owner = owner;
}



/**
 * Future::get
 * 
 * Retrieves the result of this future. 
 * Even if the future hasn't been completed yet, this function wait until 
 * it is until it returns.
 * 
 * newOwner: If not nullptr, a UniquePtr holding ownership of this Future
 *           will be placed at newOwner.
 *           If nullptr, this Future will be discarded after this call 
 *           returns.
 * 
 * Return Value: Returns the result returned by this Future's task.
 */
void *Future::get(UniquePtr<Future> *newOwner) {
    
    this->res = this->func(this->arg);

    if (newOwner != nullptr)
        *newOwner = UniquePtr<Future>(this);

    return this->res;
}



/**
 * Winpool::createNew
 * 
 * Creates and initializes a Winpool instance on the heap, starts up the 
 * worker threads, passes ownership of this instance to the caller.
 * 
 * nThreads: Number of worker threads to spawn.
 * 
 * Return Value: Returns a pointer to the newly created Winpool instance
 *               that holds ownership of it.
 */
UniquePtr<Winpool> Winpool::createNew(int nThreads) {
    UniquePtr<Winpool> pool = UniquePtr<Winpool>(new Winpool);
    pool->running = true;
    return std::move(pool);
}



/**
 * Winpool::submit
 * 
 * Adds a task to the pool's task queue. Once this is called, the pool may
 * start executing the task immediately.
 * 
 * func: Function to execute.
 * arg: Argument to pass to func.
 * 
 * Return Value: Returns a pointer to a heap-allocated Future which can
 *               be used to obtain the task's result in the future.
 *               The caller does NOT own this memory - do NOT delete: 
 *               ownership of this memory is held by the pool until 
 *               completion.
 */
Future *Winpool::submit(WinpoolTask func, void *arg) {

    Future *fut = new Future(func, arg, this, nullptr);
    return fut;
}



Future::Future(bool sentinel) {
    this->status = SENTINEL;
}



Future::~Future() {
    // Do nothing
}



Worker::Worker() {
    // Do nothing
}



FutureList::FutureList() :
            headSentinel(true),
            tailSentinel(true) {
    // Do nothing
}



FutureList::~FutureList() {
    // Do nothing
}




