
/**
 * Future.externalGet.cxx
 */



#include <cstdio>
#include <memory>
#include <windows.h>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



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
void *Future::externalGet(UniquePtr<Future> *newOwner) {

    BOOL boolRc;

    EnterCriticalSection(this->lock);

    if (this->status != DONE) {
        boolRc = SleepConditionVariableCS(
            &this->condCompleted, 
            this->lock, 
            INFINITE
        );
        if (!boolRc) {
            std::fprintf(stderr, "Future::externalGet sleep failed\n");
            std::fflush(stderr);
            throw SyscallError(GetLastError());
        }
    }

    void *res = this->res;
    UniquePtr<Future> uThis = this->popFromList();
    if (newOwner != nullptr)
        *newOwner = std::move(uThis);

    LeaveCriticalSection(this->lock);

    return res;
}
