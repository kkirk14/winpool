
/**
 * TestMain.cxx
 */



#include <cstdio>
#include "_winpool_private.hxx"
#include "_winpool_tests_private.hxx"



int main() {

    std::printf("Running testInsertHead...\n");
    std::fflush(stdout);
    testInsertHead();
    std::printf("testInsertHead succeeded\n\n");
    std::fflush(stdout);

    std::printf("Running testInsertTail...\n");
    std::fflush(stdout);
    testInsertTail();
    std::printf("testInsertTail succeeded\n\n");
    std::fflush(stdout);

    std::printf("Running testPopHead...\n");
    std::fflush(stdout);
    testPopHead();
    std::printf("testPopHead succeeded\n\n");
    std::fflush(stdout);

    std::printf("Running testPopTail...\n");
    std::fflush(stdout);
    testPopTail();
    std::printf("testPopTail succeeded\n\n");
    std::fflush(stdout);

    std::printf("Running testFuturePopFromList...\n");
    std::fflush(stdout);
    testFuturePopFromList();
    std::printf("testFuturePopFromList succeeded\n\n");
    std::fflush(stdout);
}
