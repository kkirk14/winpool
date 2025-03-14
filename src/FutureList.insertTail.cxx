
/**
 * FutureList.insertTail
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * insertTail
 * 
 * Inserts an element at the start of the list.
 * 
 * toInsert: Pointer to the element to insert with ownership passed to this
 *           function.
 */
void FutureList::insertTail(UniquePtr<Future> toInsert) {
    Future *prev = this->tailSentinel.prev;
    toInsert->next = std::move(prev->next);
    toInsert->next->prev = toInsert.get();
    toInsert->prev = prev;
    prev->next = std::move(toInsert);
}
