
/**
 * FutureList.insertHead.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * insertHead
 * 
 * Inserts an element at the start of the list.
 * 
 * toInsert: Pointer to the element to insert with ownership passed to this
 *           function.
 */
void FutureList::insertHead(UniquePtr<Future> toInsert) {
    toInsert->next = std::move(this->headSentinel.next);
    toInsert->next->prev = toInsert.get();
    toInsert->prev = &this->headSentinel;
    this->headSentinel.next = std::move(toInsert);
}
