
/**
 * Winpool.createNew.cxx
 */



#include <memory>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



/**
 * Winpool::createNew
 * 
 * Creates and initializes a Winpool instance on the heap, starts up the 
 * worker threads, passes ownership of this instance to the caller.
 * 
 * nThreads: Number of worker threads to spawn.
 * 
 * Return Value: Returns a pointer to the newly created Winpool instance
 *               that holds ownership of it.
 */
UniquePtr<Winpool> Winpool::createNew(int nThreads) {

    UniquePtr<Winpool> poolPtr = UniquePtr<Winpool>(new Winpool(nThreads));
    return std::move(poolPtr);
}
