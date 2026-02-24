#pragma once

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <cstdlib>

#include "mi_memory_pool/memory_pool.h"


static constexpr size_t _kOps = 100'000;
static constexpr size_t _kThreads = 8;

template <typename T>
inline void do_not_optimize(T&& value)
{
    // prevent compiler optimization
    asm volatile("" : : "r,m"(value) : "memory");
}

struct MallocFree
{
    static void* alloc(size_t size) { return ::malloc(size); }
    static void free(void* p, size_t) { ::free(p); }
};

struct NewDelete
{
    static void* alloc(size_t size) { return ::operator new(size); }
    static void free(void* p, size_t) { ::operator delete(p); }
};

struct MiPool
{
    static void* alloc(size_t size) { return MiMemoryPool::alloc(size); }
    static void free(void* p, size_t size) { MiMemoryPool::free(p, size); }
};

template <typename AllocFunc, typename FreeFunc>
double single_thread_test(size_t size, int iterations, AllocFunc alloc, FreeFunc free)
{
    for (int i = 0; i < WARMUP_ITERATIONS; ++i) {
        void* p = alloc(size);
        do_not_optimize(p);
        free(p, size);
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < iterations; i++) {
        void* p = alloc(size);
        do_not_optimize(p);
        free(p, size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

template <typename AllocFunc, typename FreeFunc>
double multi_thread_test(size_t size, int iterations, int n_threads, AllocFunc alloc, FreeFunc free)
{
    auto worker = [&]{
        for (size_t i = 0; i < iterations; i++) {
            void* p = alloc(size);
            do_not_optimize(p);
            free(p, size);
        }
    };

    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < n_threads; i++)
        threads.emplace_back(worker);
    for (auto& t : threads)
        t.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count();
}

