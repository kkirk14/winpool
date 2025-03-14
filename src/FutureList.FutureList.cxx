
/**
 * FutureList.FutureList.cxx
 * 
 * Contains definitions for the FutureList constructors.
 */



#include "_winpool_private.hxx"
#include <memory>



using namespace WinpoolNS;



/**
 * FutureList constructor
 * 
 * Creates an empty Future list with 2 sentinel nodes linked to each other.
 */
FutureList::FutureList() : 
            headSentinel(true), 
            tailSentinel(true) {
    
    // Link the sentinel nodes to each other
    this->headSentinel.next = UniquePtr<Future>(&this->tailSentinel);
    this->headSentinel.prev = nullptr;
    this->tailSentinel.next = UniquePtr<Future>(nullptr);
    this->tailSentinel.prev = &this->headSentinel;
}
