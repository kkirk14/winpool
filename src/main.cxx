
/**
 * main
 * 
 * Just some testing stuff.
 */



#include <memory>
#include <cstdio>
#include "_winpool_private.hxx"



using WinpoolNS::FutureList;
using WinpoolNS::Future;
using WinpoolNS::Winpool;
using WinpoolNS::UniquePtr;
using WinpoolNS::SyscallError;



static void printFutureList(FutureList *futList) {

    for (Future *fut = futList->headSentinel.next.get(); 
         fut->next != nullptr;
         fut = fut->next.get()) {
        
        std::printf("%d\n", fut->arg);
    }
}



void *sayHello(void *name) {
    std::printf("hello %s!\n", (char *)name);
    std::fflush(stdout);
    return (void *)1;
}



/**
 * main
 * 
 * Execution starts here.
 */

int main() {

    UniquePtr<Winpool> pool;

    try {
        pool = Winpool::createNew(8);
    }
    catch (SyscallError e) {
        std::fprintf(stderr, "Winpool::createNew syscall error\n");
        std::fflush(stderr);
        return 0;
    }

    UniquePtr<Future> fut;
    Future *bFut = pool->submit(sayHello, (void *)"Kevin");
    void *pvRes = bFut->get(&fut);
    std::printf(
        "Task completed\n"
        "Received result: %d\n"
        "Executed by worker %lu\n",
        pvRes, 
        fut->executor - pool->workers.get()
    );

    return 0;
}
