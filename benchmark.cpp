#include <iostream>

#include "./benchmark.h"


const std::vector<size_t> SIZES = {8, 16, 32, 64, 128, 256};

void print_result(const std::string& name, size_t size, double seconds, int ops, int threads) {
    double throughput = ops / seconds;
    std::cout << name << "\t" << size << "\t" << threads << "\t"
              << seconds << " s\t" << throughput << " ops/s\n";
}

int main()
{
    std::cout << "Allocator\tSize\tThreads\tTime(s)\tThroughput(ops/s)\n";

    /* Single Thread */
    for (size_t size : SIZES) {
        // malloc
        double t = single_thread_test(size, _kOps, MallocFree::alloc, MallocFree::free);
        print_result("malloc/free", size, t, _kOps, 1);

        // operator new
        t = single_thread_test(size, _kOps, NewDelete::alloc, NewDelete::free);
        print_result("new/delete", size, t, _kOps, 1);

        // MiMemoryPool
        t = single_thread_test(size, _kOps, MiPool::alloc, MiPool::free);
        print_result("MiMemoryPool", size, t, _kOps, 1);
    }

    /* Multi Thread */
    for (size_t size : SIZES) {
        // malloc
        double t = multi_thread_test(size, _kOps, _kThreads, MallocFree::alloc, MallocFree::free);
        print_result("malloc/free", size, t, _kOps, _kThreads);

        // operator new
        t = multi_thread_test(size, _kOps, _kThreads, NewDelete::alloc, NewDelete::free);
        print_result("new/delete", size, t, _kOps, _kThreads);

        // MiMemoryPool
        t = multi_thread_test(size, _kOps, _kThreads, MiPool::alloc, MiPool::free);
        print_result("MiMemoryPool", size, t, _kOps, _kThreads);
    }

    return 0;
}