
/**
 * FutureList.popTail.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * popTail
 * 
 * Removes and returns the last element in the list.
 * 
 * Return Value: Returns a pointer to the popped Future. This pointer owns
 *               the Future's memory.
 *               Returns nullptr if the list is empty.
 */
UniquePtr<Future> FutureList::popTail() {
    
    if (this->empty()) {
        return UniquePtr<Future>(nullptr);
    }

    Future *prev = this->tailSentinel.prev->prev;
    UniquePtr<Future> popped = std::move(prev->next);
    popped->next->prev = prev;
    prev->next = std::move(popped->next);
    return std::move(popped);
}
