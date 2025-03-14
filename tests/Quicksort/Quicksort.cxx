
/**
 * Quicksort.cxx
 * 
 * Tests the Winpool library with a parallel quicksort implementation.
 */



#include <memory>
#include <cassert>
#include <chrono>
#include <inttypes.h>
#include <cstdlib>
#include <cstdio>
#include <winpool.hxx>



using namespace std::chrono;
using namespace WinpoolNS;


/* BILLION: 1-billion long long literal. */
#define BILLION 1000000000LL



static void printArr(int32_t *arr, size_t len_arr) {
    std::printf("[");
    for (int iElem = 0; iElem < len_arr; iElem++) {
        std::printf("%d", arr[iElem]);
        if (iElem < len_arr - 1)
            std::printf(", ");
    }
    std::printf("]\n");
}



static bool isSorted(int32_t *bpArr, size_t len_arr) {
    for (int iElem = 0; iElem < len_arr - 1; iElem++) {
        if (bpArr[iElem] > bpArr[iElem + 1])
            return false;
    }
    return true;
}



/**
 * quicksortSeq
 * 
 * Sequential implementation of quicksort.
 * 
 * arr: Entire array being sorted.
 * iStart: Index into arr at which our segment to sort starts (inclusive).
 * iEnd: Index into arr at which our segment to sort starts (exclusive).
 */
static void quicksortSeq(int32_t *arr, size_t iStart, size_t iEnd) {

    if (iStart >= iEnd)
        return;

    int32_t pivot = arr[iStart];

    size_t iLeft = iStart + 1;
    size_t iRight = iEnd - 1;

    while (iLeft <= iRight) {

        // Find the elements that need to be swapped
        while (iLeft <= iRight && arr[iLeft] < pivot) {
            iLeft++;
        }
        while (iRight >= iLeft && arr[iRight] >= pivot) {
            iRight--;
        }

        // Perform the swap
        if (iLeft < iRight) {
            int32_t tmp = arr[iLeft];
            arr[iLeft] = arr[iRight];
            arr[iRight] = tmp;
            iLeft++;
            iRight--;
        }
    }

    // Put the pivot in the proper segment
    if (iRight > iStart) {
        arr[iStart] = arr[iRight];
        arr[iRight] = pivot;
    }

    // iRight now points to the first element in the right segment

    // Make the recursive calls on the left and right segments
    quicksortSeq(arr, iStart, iRight);
    quicksortSeq(arr, iRight + 1, iEnd);
}



class QuicksortArgs final {
public:
    Winpool *bpPool;
    int32_t *arr;
    size_t iStart;
    size_t iEnd;

    QuicksortArgs(Winpool *bpPool, int32_t *arr, size_t iStart, size_t iEnd);
};


QuicksortArgs::QuicksortArgs(Winpool *bpPool, 
                             int32_t *arr, 
                             size_t iStart, 
                             size_t iEnd) {

    this->bpPool = bpPool;
    this->arr = arr;
    this->iStart = iStart;
    this->iEnd = iEnd;
}



static void *quicksortParallel(void *_arg) {

    UniquePtr<QuicksortArgs> args((QuicksortArgs *)_arg);

    //Worker *bpMyWorker = (Worker *)TlsGetValue(args->bpPool->workerTlsIdx);
    //
    //std::printf(
    //    "worker %lu: quickSortParallel(%lu, %lu)\n",
    //    bpMyWorker - args->bpPool->workers.get(),
    //    args->iStart, 
    //    args->iEnd);
    //std::fflush(stdout);

    if (args->iEnd - args->iStart <= 4096) {
        quicksortSeq(args->arr, args->iStart, args->iEnd);
        return nullptr;
    }

    int32_t pivot = args->arr[args->iStart];

    size_t iLeft = args->iStart + 1;
    size_t iRight = args->iEnd - 1;

    while (iLeft <= iRight) {

        // Find the elements that need to be swapped
        while (iLeft <= iRight && args->arr[iLeft] < pivot) {
            iLeft++;
        }
        while (iRight >= iLeft && args->arr[iRight] >= pivot) {
            iRight--;
        }

        // Perform the swap
        if (iLeft < iRight) {
            int32_t tmp = args->arr[iLeft];
            args->arr[iLeft] = args->arr[iRight];
            args->arr[iRight] = tmp;
            iLeft++;
            iRight--;
        }
    }

    // Put the pivot in the proper spot
    if (iRight > args->iStart) {
        args->arr[args->iStart] = args->arr[iRight];
        args->arr[iRight] = pivot;
    }

    // iRight now points to the first element in the right segment

    // Make the recursive calls on the left and right segments
    Future *bpRightFut = args->bpPool->submit(
        quicksortParallel,
        new QuicksortArgs(
            args->bpPool, 
            args->arr, 
            iRight + 1, 
            args->iEnd
        )
    );
    quicksortParallel(
        new QuicksortArgs(
            args->bpPool, 
            args->arr, 
            args->iStart, 
            iRight
        )
    );
    
    // Join the subtask
    UniquePtr<Future> rightFut;
    bpRightFut->get(&rightFut);

    return nullptr;
}



#define GIBI (1LL << 30)

#define MEBI (1LL << 20)



/* ARR_SIZE: Number of elements in array to be sorted. */
#define ARR_SIZE 40 * MEBI



/**
 * main
 * 
 * Execution starts here.
 */
int main() {
    using Duration = high_resolution_clock::duration;
    using TimePoint = high_resolution_clock::time_point;
    using Nanoseconds = nanoseconds;

    assert(sizeof(int) >= sizeof(int32_t));

    // Set random seed
    std::srand(
        (unsigned int)high_resolution_clock::now()
                                            .time_since_epoch()
                                            .count()
    );

    // Create threadpool
    UniquePtr<Winpool> pool = Winpool::createNew(16);

    const int N_TRIALS = 10;
    // const int len_arr = 10;

    for (int iTrial = 0; iTrial < N_TRIALS; iTrial++) {

        // Create array
        UniquePtr<int32_t[]> arr(new int32_t[ARR_SIZE]);
        for (int iElem = 0; iElem < ARR_SIZE; iElem++) {
            arr[iElem] = (int32_t)(std::rand() & ((1LL << 31) - 1));
        }

        // Print original array
        std::printf("Original arr:\t");
        printArr(arr.get(), 20);

        // Start timer
        std::printf("Running trial %d...\n", iTrial);
        std::fflush(stdout);
        TimePoint start = high_resolution_clock::now();

        // Sort array
        UniquePtr<Future> fut;
        Future *bpFut = pool->submit(
            quicksortParallel,
            new QuicksortArgs(pool.get(), arr.get(), 0, ARR_SIZE) 
        );
        bpFut->get(&fut);

        // Stop timer
        TimePoint end = high_resolution_clock::now();
        Duration runTime = end - start;
        Nanoseconds nsRunTime = duration_cast<Nanoseconds>(runTime);
        double secs = ((double)nsRunTime.count()) / ((double)BILLION);

        // Print sorted array
        std::printf("Sorted arr:\t");
        printArr(arr.get(), 20);
        assert(isSorted(arr.get(), ARR_SIZE));
        std::printf("Passed in %lf seconds!\n", secs);
        std::printf("\n");
        std::fflush(stdout);
    }
}
