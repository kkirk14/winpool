
/**
 * TestFutureList.cxx
 */



#include <memory>
#include <cassert>
#include <cstdio>
#include "_winpool_private.hxx"



using namespace WinpoolNS;



void testInsertHead() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    FutureList futList;

    UniquePtr<Future> f1 = 
        UniquePtr<Future>(new Future(nullptr, (void *)1, nullptr));
    UniquePtr<Future> f2 = 
        UniquePtr<Future>(new Future(nullptr, (void *)2, nullptr));
    UniquePtr<Future> f3 = 
        UniquePtr<Future>(new Future(nullptr, (void *)3, nullptr));

    // List should be 3, 2, 1
    futList.insertHead(std::move(f1));
    futList.insertHead(std::move(f2));
    futList.insertHead(std::move(f3));

    int n = 3;
    for (Future *fut = futList.headSentinel.next.get(); 
         fut->next != nullptr;
         fut = fut->next.get()) {
        
        assert((void *)n == fut->arg);
        n--;
    }

#pragma GCC diagnostic pop
}



void testInsertTail() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    FutureList futList;

    UniquePtr<Future> f1 = 
        UniquePtr<Future>(new Future(nullptr, (void *)1, nullptr));
    UniquePtr<Future> f2 = 
        UniquePtr<Future>(new Future(nullptr, (void *)2, nullptr));
    UniquePtr<Future> f3 = 
        UniquePtr<Future>(new Future(nullptr, (void *)3, nullptr));

    // List is 1, 2, 3
    futList.insertTail(std::move(f1));
    futList.insertTail(std::move(f2));
    futList.insertTail(std::move(f3));

    int n = 1;
    for (Future *fut = futList.headSentinel.next.get();
         fut->next != nullptr;
         fut = fut->next.get()) {

        assert((void *)n == fut->arg);
        n++;
    }

#pragma GCC diagnostic pop
}



void testPopHead() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    FutureList futList;

    UniquePtr<Future> f1 = 
        UniquePtr<Future>(new Future(nullptr, (void *)1, nullptr));
    UniquePtr<Future> f2 = 
        UniquePtr<Future>(new Future(nullptr, (void *)2, nullptr));
    UniquePtr<Future> f3 = 
        UniquePtr<Future>(new Future(nullptr, (void *)3, nullptr));

    // List is 1, 2, 3
    futList.insertTail(std::move(f1));
    futList.insertTail(std::move(f2));
    futList.insertTail(std::move(f3));

    int n = 1;
    for (UniquePtr<Future> popped = futList.popHead();
         popped != nullptr;
         popped = futList.popHead()) {
        
        assert((void *)n == popped->arg);
        n++;
    }

#pragma GCC diagnostic pop
}



void testPopTail() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    FutureList futList;

    UniquePtr<Future> f1 = 
        UniquePtr<Future>(new Future(nullptr, (void *)1, nullptr));
    UniquePtr<Future> f2 = 
        UniquePtr<Future>(new Future(nullptr, (void *)2, nullptr));
    UniquePtr<Future> f3 = 
        UniquePtr<Future>(new Future(nullptr, (void *)3, nullptr));

    // List is 1, 2, 3
    futList.insertTail(std::move(f1));
    futList.insertTail(std::move(f2));
    futList.insertTail(std::move(f3));

    int n = 3;
    for (UniquePtr<Future> popped = futList.popTail();
         popped != nullptr;
         popped = futList.popTail()) {
        
        assert((void *)n == popped->arg);
        n--;
    }

#pragma GCC diagnostic pop
}



void testFuturePopFromList() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

    FutureList futList;

    UniquePtr<Future> oldFut1 = 
        UniquePtr<Future>(new Future(nullptr, (void *)1, nullptr));
    UniquePtr<Future> oldFut2 = 
        UniquePtr<Future>(new Future(nullptr, (void *)2, nullptr));
    UniquePtr<Future> oldFut3 = 
        UniquePtr<Future>(new Future(nullptr, (void *)3, nullptr));

    Future *bFut1 = oldFut1.get();
    Future *bFut2 = oldFut2.get();
    Future *bFut3 = oldFut3.get();

    // List is 1, 2, 3
    futList.insertTail(std::move(oldFut1));
    futList.insertTail(std::move(oldFut2));
    futList.insertTail(std::move(oldFut3));

    UniquePtr<Future> newFut2 = bFut2->popFromList();
    assert(bFut1->next.get() == bFut3);
    assert(bFut3->prev = bFut1);

    UniquePtr<Future> newFut1 = bFut1->popFromList();
    assert(futList.headSentinel.next.get() == bFut3);
    assert(futList.tailSentinel.prev == bFut3);

    UniquePtr<Future> newFut3 = bFut3->popFromList();
    assert(futList.empty());

    assert(newFut1->arg == (void *)1);
    assert(newFut2->arg == (void *)2);
    assert(newFut3->arg == (void *)3);

#pragma GCC diagnostic pop
}
