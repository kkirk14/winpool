
/**
 * Winpool.Winpool.cxx
 */



#include <memory>
#include <iso646.h>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * WorkerTProcData constructor
 * 
 * Just sets the members.
 */
WorkerTProcData::WorkerTProcData(Winpool *pool, 
                                 DWORD tlsMyWorkerIdx, 
                                 Worker *myWorker) {

    this->pool = pool;
    this->tlsMyWorkerIdx = tlsMyWorkerIdx;
    this->myWorker = myWorker;
}



/**
 * Winpool constructor
 * 
 * Initializes a new Winpool instance and starts up its worker threads.
 * 
 * nThreads: The number of worker threads to spawn.
 */
Winpool::Winpool(int nThreads) :
         futures() {

    BOOL boolRc;
    DWORD errorCode;
    int iWorker = 0;
    
    this->nWorkers = nThreads;
    this->lock = &this->futures.lock;
    this->hWorkerThreads = UniquePtr<HANDLE[]>(new HANDLE[nThreads]);
    this->workers = UniquePtr<Worker[]>(new Worker[nThreads]);
    this->workerDatas = UniquePtr<WorkerTProcData[]>(
        (WorkerTProcData *)malloc(sizeof(WorkerTProcData) * nThreads)
    );

    // Allocate Tls worker variable
    DWORD tlsMyWorkerIdx = TlsAlloc();
    if (tlsMyWorkerIdx < 0) {
        errorCode = GetLastError();
        goto onError;
    }
    boolRc = TlsSetValue(tlsMyWorkerIdx, nullptr);
    if (!boolRc) {
        errorCode = GetLastError();
        goto onError;
    }
    this->workerTlsIdx = tlsMyWorkerIdx;

    // Start the worker threads
    for (iWorker = 0; iWorker < nThreads; iWorker++) {
        this->workerDatas[iWorker] = WorkerTProcData(
            this, 
            tlsMyWorkerIdx,
            this->workers.get() + iWorker
        );
        this->hWorkerThreads[iWorker] = CreateThread(
            NULL,
            0,
            workerTProc,
            (LPVOID)(&this->workerDatas.get()[iWorker]),
            0,
            NULL
        );
        if (this->hWorkerThreads[iWorker] == INVALID_HANDLE_VALUE) {
            errorCode = GetLastError();
            goto onError;
        }
    }

    this->running = true;

    return;

onError:
    for (int i = 0; i < iWorker; i++) {
        TerminateThread(this->hWorkerThreads[i], errorCode);
        WaitForSingleObject(this->hWorkerThreads[i], INFINITE);
        CloseHandle(this->hWorkerThreads[i]);
    }
    this->hWorkerThreads.reset(nullptr);
    this->workers.reset(nullptr);
    this->workerDatas.reset(nullptr);
    TlsFree(tlsMyWorkerIdx);
    throw SyscallError(errorCode);
}
