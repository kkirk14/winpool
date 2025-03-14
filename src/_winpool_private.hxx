
/**
 * _winpool_private.h
 * 
 * Function and static-duration variable declarations for use within 
 * winpool library code.
 */



#ifndef _WINPOOL_PRIVATE_H
#define _WINPOOL_PRIVATE_H



#include <windows.h>
#include <memory>
#include <functional>



/**
 * Winpool namespace
 */
namespace WinpoolNS {


/* Forward class declarations */
class Worker;
class Future;
class FutureList;
class Winpool;

/* FutureOwner: Winpool class needs the same members as worker, so we'll
                call it a FutureOwner there. */
typedef Worker FutureOwner;

/* UniquePtr: Looks better than std::unique_ptr */
template<class T> 
using UniquePtr = std::unique_ptr<T>;

/* WinpoolTask: Cleaner name for void *(void *) */
using WinpoolTask = std::function<void *(void *)>;


/* spinCount: All locks in the pool (workers and pool) will use this spin 
              count. */
const DWORD spinCount = 50;



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
DWORD WINAPI workerTProc(void *arg);



/**
 * SyscallError class
 * 
 * Exception class that is thrown on syscall failures.
 */
class SyscallError final {
public:

    /* error: Error code returned by GetLastError. */
    DWORD error;


    /**
     * SyscallError constructor
     * 
     * error: Error code returned by GetLastError
     */
    SyscallError(DWORD error);
};



/**
 * WorkerTProcData class
 * 
 * Data that is passed to the worker threads by the pool on worker thread 
 * startup.
 */
class WorkerTProcData final {
public:

    Winpool *pool;

    DWORD tlsMyWorkerIdx;

    Worker *myWorker;


    /**
     * WorkerTProcData constructor
     * 
     * Justs sets the members.
     */
    WorkerTProcData(Winpool *pool, DWORD tlsMyWorkerIdx, Worker *myWorker);
};


/**
 * FutureStatus enum
 */
typedef enum _FutureStatus {
    QUEUED,
    RUNNING,
    DONE,
    SENTINEL // Only used for FutureList stuff
} FutureStatus;



/**
 * Future class
 */
class Future final {
public:
    
    /* lock: This protects all memory in this class. This will point to the lock
             protecting the queue this Future was put on (either pool or worker
             lock).
             Lock this everytime you access this Future's memory. */
    CRITICAL_SECTION *lock;
    
    /* status: Use this to determine what stage of execution the future is in. */
    FutureStatus status;

    /* bPool: Borrowed pointer to the Winpool this Future was submitted to and
              is run on. */
    Winpool *bPool;
    
    /* func: This functor will be executed. */
    WinpoolTask func;
    
    /* arg: this will be passed to func. */
    void *arg; 
    
    /* res: Result returned from func. */
    void *res; 
    
    /* owner: Points to the worker whose queue this future was placed on - lock
              points to owner's lock. */
    FutureOwner *owner;
    
    /* executor: Points to the worker who executed/is executing this task. */
    Worker *executor;
    
    /* condCompleted: Condition variable will be broadcasted when the future
                      is fulfilled and the result is available. Used by 
                      external threads calling Future.get. */
    CONDITION_VARIABLE condCompleted;
    
    /* next: Points to the next element in whatever FutureList this is in. 
             This pointer holds ownership of the Future it points to. */
    UniquePtr<Future> next;
    
    /* prev: Borrowing pointer to the previous element in whatever FutureList 
             this is in. */
    Future *prev;
    
    
    /**
     * Future constructor
     * 
     * func: Task to execute.
     * arg: Argument to pass to func.
     * bPool: Borrowed pointer to the pool.
     * owner: Borrowed pointer to the FutureOwner that protects this Future.
     */
    Future(WinpoolTask func, void *arg, Winpool *bPool, FutureOwner *owner);
    
    /** 
     * Future sentinel constructor
     * 
     * Passing a bool to the Future constructor will initialize it as a
     * sentinel node for use in a FutureList.
     */
    Future(bool sentinel);
    
    /**
     * Future destructor
     */
    ~Future();
    
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
    void *get(UniquePtr<Future> *newOwner);

    /**
     * Future::removeFromList
     * 
     * Removes this future from the FutureList its currently in gives ownership
     * of this Future's memory to the caller.
     * 
     * Return Value: Returns a pointer to this that has ownership of this 
     *               Future's memory.
     *               Returns nullptr if this Future isn't in a list.
     */
    UniquePtr<Future> popFromList();
    
private:
    
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
    void *workerGet(UniquePtr<Future> *newOwner, Worker *bMyWorker);
    
    /**
     * Future::externalGet
     * 
     * Helper function for Future::get - only called by external (non-worker) 
     * threads.
     * Blocks the calling thread until the Future is DONE.DONE
     * 
     * newOwner: Ownership of this Future will be passed here if it's not
     *           nullptr.
     * 
     * Return Value: Returns the result returned by this Future's task.
     */
    void *externalGet(UniquePtr<Future> *newOwner);
};



/**
 * FutureList class
 */
class FutureList final {
public:

    /* headSentinel: Dummy future that will point to the first list element. */
    Future headSentinel;

    /* tailSentinel: Dummy future that will point to the last list element. */
    Future tailSentinel;


    /**
     * FutureList constructor
     */
    FutureList();

    /**
     * FutureList destructor
     * 
     * On destruction, all Futures in this list will be deleted.
     */
    ~FutureList();

    /**
     * popHead
     * 
     * Removes and returns the first element in the list.
     * 
     * Return Value: Returns a pointer to the popped Future. This pointer owns
     *               the Future's memory.
     */
    UniquePtr<Future> popHead();

    /**
     * popTail
     * 
     * Removes and returns the last element in the list.
     * 
     * Return Value: Returns a pointer to the popped Future. This pointer owns
     *               the Future's memory.
     *               Returns nullptr if the list is empty.
     */
    UniquePtr<Future> popTail();

    /**
     * insertHead
     * 
     * Inserts an element at the start of the list.
     * 
     * toInsert: Pointer to the element to insert with ownership passed to this
     *           function.
     */
    void insertHead(UniquePtr<Future> toInsert);

    /**
     * insertTail
     * 
     * Inserts an element at the start of the list.
     * 
     * toInsert: Pointer to the element to insert with ownership passed to this
     *           function.
     */
    void insertTail(UniquePtr<Future> toInsert);

    /**
     * empty
     * 
     * Is there anything in the list?
     * 
     * Return Value: Returns true if the list has no elements and pop will
     *               return nullptr. Returns false if list has >= 1 element.
     */
    bool empty();
};



/**
 * Worker class
 */
class Worker final {
public:

    /* lock: Protects all members of this class, its queues, and all 
             Futures originally inserted into our queue. */
    CRITICAL_SECTION lock;

    /* taskQueue: Linked list that contains subtasks of tasks we're executing
                  that need to be executed. */
    FutureList taskQueue;

    /* completedList: Linked list that just holds ownership of completed 
                      Futures we own until the user takes ownership with 
                      Future::get. */
    FutureList completedList;

    /**
     * Worker constructor
     * 
     * Initializes this instance with an empty taskQueue and completedList.
     */
    Worker();
};



/**
 * Winpool class
 */
class Winpool final {
public:

    /* futures: Pool queue of tasks and list holding ownership of completed 
                tasks. */
    FutureOwner futures;

    /* nWorkers: The number of worker threads. This is the size of the 
                 hWorkerThreads and the workers array. */
    int nWorkers;

    /* workerIdTlsIdx: Tls index of the thread-local worker object. 
                       This will be nullptr in external threads. */
    DWORD workerTlsIdx;

    /* lock: This protects all data in this class and the pool's task queue.
             Note: This will point to this->futures.lock. */
    CRITICAL_SECTION *lock;

    /* hWorkerThreads: Points to a heap-allocated array of the worker threads'
                       HANDLEs. */
    UniquePtr<HANDLE[]> hWorkerThreads;

    /* workers: Points to heap-allocated array of Worker instances. Each worker
                contains data needed for a worker thread. */
    UniquePtr<Worker[]> workers;

    /* workerDatas: Points to heap-allocated array of Worker instances */
    UniquePtr<WorkerTProcData[]> workerDatas;

    bool running;


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
    static UniquePtr<Winpool> createNew(int nThreads);


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
    Future *submit(WinpoolTask func, void *arg);

    /**
     * Winpool::shutdown
     */
    bool shutdown();

    /**
     * Winpool destructor
     */
    // ~Winpool();

private:

    /**
     * Winpool constructor
     * 
     * Initializes a new Winpool instance and starts up its worker threads.
     * 
     * nThreads: The number of worker threads to spawn.
     */
    Winpool(int nThreads);

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
    Future *workerSubmit(WinpoolTask func, void *arg, Worker *worker);

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
    Future *externalSubmit(WinpoolTask func, void *arg);
};

} // end WinpoolNS



#endif // ifdef _WINPOOL_PRIVATE_H
