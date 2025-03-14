
/**
 * FutureList.~FutureList.cxx
 * 
 * Contains FutureList destructor definition.
 */



#include <memory>
#include "_winpool_private.hxx"



/**
 * FutureList destructor
 * 
 * On destruction, all Futures in this list will be deleted.
 */
WinpoolNS::FutureList::~FutureList() {

    // Iterate through the list, removing nodes
    // We need to iterate like this because default FutureList destructor will
    // call individual Future destructors recursively (potential overflow).
    UniquePtr<Future> currFut;
    for (currFut = std::move(this->headSentinel.next);
         currFut->next != nullptr;
         currFut = std::move(currFut->next)) { }

    // At this point, currFut has ownership of tailSentinel - we need to 
    // release it since tailSentinel doesn't have its own heap block.
    currFut.release();
}
