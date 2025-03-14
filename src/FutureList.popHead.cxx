
/**
 * FutureList.popHead.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * popHead
 * 
 * Removes and returns the first element in the list.
 * 
 * Return Value: Returns a pointer to the popped Future. This pointer owns
 *               the Future's memory.
 */
UniquePtr<Future> FutureList::popHead() {

    if (this->empty()) {
        return UniquePtr<Future>(nullptr);
    }
    
    UniquePtr<Future> popped = std::move(this->headSentinel.next);
    popped->next->prev = popped->prev;
    popped->prev->next = std::move(popped->next);
    return std::move(popped);
}
