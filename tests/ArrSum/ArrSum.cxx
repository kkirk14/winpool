
/**
 * ArrSum.cxx
 * 
 * Tests the Winpool library with a array summing task.
 */



#include <memory>
#include <cstdio>
#include <chrono>
#include <inttypes.h>
#include <winpool.hxx>



using namespace WinpoolNS;
using namespace std::chrono;



/**
 * SumArrArgs
 * 
 * Class that contains data passed to sumArr.
 */
class SumArrArgs {
public:

    /* bPool: Borrowed pointer to the Winpool this task is running on. */
    Winpool *bPool;

    /* arr: Pointer to the integer array to sum from. */
    int32_t *arr;

    /* iStart: Start of the segment to sum (inclusive). */
    size_t iStart; 

    /* iEnd: End of the segment to sum (exclusive). */
    size_t iEnd;

    SumArrArgs(Winpool *bPool, int32_t *arr, size_t iStart, size_t iEnd);
};


/**
 * SumArrArgs constructor
 */
SumArrArgs::SumArrArgs(Winpool *bPool, 
                       int32_t *arr, 
                       size_t iStart, 
                       size_t iEnd) {

    this->bPool = bPool;
    this->arr = arr;
    this->iStart = iStart;
    this->iEnd = iEnd;
}



/**
 * sumArr
 * 
 * Winpool task that recursively finds the sum of an array.
 */
void *sumArr(void *_arg) {

    UniquePtr<SumArrArgs> args = UniquePtr<SumArrArgs>(
        (SumArrArgs *)_arg
    );

    // Base case: Small segment
    if (args->iEnd - args->iStart < 1000) {
        int64_t sum = 0;
        for (size_t i = args->iStart; i < args->iEnd; i++) {
            sum += args->arr[i];
        }
        return (void *)sum;
    }

    // Find the midpoint
    size_t iMid = (args->iStart + args->iEnd) / 2;
    
    // Submit the task to sum the second half
    Future *bHalf2Fut = args->bPool->submit(
        sumArr, 
        new SumArrArgs(args->bPool, args->arr, iMid, args->iEnd)
    );

    // Sum the first half
    int64_t half1Sum = (int64_t)sumArr(
        new SumArrArgs(args->bPool, args->arr, args->iStart, iMid)
    );

    // Join the second half Future
    UniquePtr<Future> half2Fut;
    int64_t half2Sum = (int64_t)bHalf2Fut->get(&half2Fut);

    // Return the sum
    return (void *)(half1Sum + half2Sum);
}



#define BILLION 1000000000LL



static const size_t len_arr = 6 * BILLION;


/**
 * main
 * 
 * Execution starts here.
 */
int main() {

    using TimePoint = high_resolution_clock::time_point;
    using Duration = high_resolution_clock::duration;
    using Nanoseconds = std::chrono::nanoseconds;

    UniquePtr<Winpool> pool;
    try {
        pool = Winpool::createNew(16);
    }
    catch (SyscallError e) {
        std::fprintf(stderr, "syscall failure Winpool::createNew\n");
        std::fflush(stderr);
        return 1;
    }

    int32_t *arr = new int32_t[len_arr];
    for (int i = 0; i < len_arr; i++) {
        arr[i] = 1;
    }

    std::printf("submitting future...\n");
    std::fflush(stdout);

    // Start the stopwatch
    TimePoint start = high_resolution_clock::now();

    // Submit/run the task
    UniquePtr<Future> fut;
    Future *bFut = pool->submit(
        sumArr, 
        new SumArrArgs(pool.get(), arr, 0, len_arr)
    );

    // Join the task and get the result
    int64_t sum = (int64_t)bFut->get(&fut);

    // Stop the stopwatch
    TimePoint end = high_resolution_clock::now();
    Duration runTime = end - start;
    Nanoseconds nsRunTime = duration_cast<Nanoseconds>(runTime);
    double secs = ((double)nsRunTime.count()) / (double)BILLION;

    // Print results
    std::printf("Received sum: %ld\n", sum);
    std::printf("nanoseconds: %lu\n", nsRunTime.count());
    std::printf("seconds: %lf\n", secs);

    return 0;
}
