#pragma once

#include <stddef.h>
#include <mutex>


namespace MemoryPool
{
    static constexpr size_t kMaxSmallSize = 128;  // Byte
    static constexpr size_t kClassGrid = 16;
    static constexpr size_t kNumClasses = kMaxSmallSize / kClassGrid;
    static constexpr size_t kFetchTime = 32;

    inline size_t idx(size_t sz) {
        size_t aligned = (sz + kClassGrid - 1) / kClassGrid;
        if (aligned == 0) aligned = 1;
        return aligned - 1;
    }
}


struct FreeNode
{
    FreeNode *next;
};

class FreeList
{
public:
    FreeList() { head = nullptr; }

    void push(FreeNode *node) {
        // head insertion
        node->next = head;
        head = node;
    }

    FreeNode* pop() {
        FreeNode *ret = head;
        head = head->next;
        return ret;
    }

    bool empty() { return head == nullptr; }

private:
    FreeNode *head;
};


class GlobalPool
{
public:
    FreeNode* alloc(size_t size);
    void free(FreeNode* node, size_t size);

private:
    FreeList pool_[MemoryPool::kNumClasses];
    std::mutex mtx_;
};


class ThreadCache
{
public:
    FreeNode* alloc(size_t size, GlobalPool& globalPool);
    void free(FreeNode* node, size_t size);

private:
    FreeList freelists_[MemoryPool::kNumClasses];
    static FreeNode* sys_alloc(size_t size);
};


class MiMemoryPool
{
public:
    static void* alloc(size_t size);
    static void free(void* ptr, size_t size);

private:
    static GlobalPool globalPool_;

    // allocate thread local storage cache
    static ThreadCache& tls_cache();
};

inline GlobalPool MiMemoryPool::globalPool_;
