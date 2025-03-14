
/**
 * Future.popFromList.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



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
UniquePtr<Future> Future::popFromList() {

    if (this->next.get() == nullptr 
         || this->prev == nullptr) {
        return UniquePtr<Future>(nullptr);
    }

    Future *prev = this->prev;
    UniquePtr<Future> thisOwner = std::move(prev->next);
    thisOwner->next->prev = prev;
    prev->next = std::move(thisOwner->next);
    return std::move(thisOwner);
}
