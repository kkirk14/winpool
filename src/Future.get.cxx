
/**
 * Future.get.cxx
 */



#include <cstdio> // DELETE LATER
#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



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

    Worker *bpMyWorker = (Worker *)TlsGetValue(this->bPool->workerTlsIdx);

    if (bpMyWorker == nullptr) {
        return this->externalGet(newOwner);
    }
    else {
        return this->workerGet(newOwner, bpMyWorker);
    }
}
