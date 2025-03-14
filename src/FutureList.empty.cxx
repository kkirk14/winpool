
/**
 * FutureList.empty.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * empty
 * 
 * Is there anything in the list?
 * 
 * Return Value: Returns true if the list has no elements and pop will
 *               return nullptr. Returns false if list has >= 1 element.
 */
bool FutureList::empty() {
    return this->headSentinel.next.get() == &this->tailSentinel;
}
