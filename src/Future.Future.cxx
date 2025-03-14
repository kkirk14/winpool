
/**
 * Future.Future.cxx
 * 
 * Contains definitions for Future constructors.
 */



#include <windows.h>
#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Future constructor
 */
Future::Future(WinpoolTask func, 
               void *arg, 
               Winpool *bPool,
               FutureOwner *owner) {
    
    this->status = QUEUED;
    this->func = func;
    this->arg = arg;
    this->owner = owner;
    this->lock = &owner->lock;
    this->executor = nullptr;
    this->bPool = bPool;

    InitializeConditionVariable(&this->condCompleted);
}



/**
 * Future sentinel constructor
 * 
 * Creates an empty Future list with 2 sentinel nodes linked to each other.
 */
Future::Future(bool sentinel) {
    status = SENTINEL;
    this->arg = (void *)15042;
}
