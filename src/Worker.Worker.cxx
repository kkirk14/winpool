
/**
 * Worker.Worker.cxx
 */



#include <windows.h>
#include <iso646.h>
#include <cstdio> // DELETE LATER
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Worker constructor
 * 
 * Initializes this instance with an empty taskQueue and completedList.
 */
Worker::Worker() : 
        taskQueue(),
        completedList() {

    BOOL boolRc;

    boolRc = InitializeCriticalSectionAndSpinCount(
        &this->lock,
        spinCount
    );
    if (not boolRc) {
        throw SyscallError(GetLastError());
    }
}
