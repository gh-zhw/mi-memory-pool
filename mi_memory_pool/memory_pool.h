#pragma once

#include <stddef.h>
#include <mutex>


namespace MemoryPool
{
    static constexpr size_t kMaxSmallSize = 128;  // Byte
    static constexpr size_t kClassGrid = 16;
    static constexpr size_t kNumClasses = kMaxSmallSize / kClassGrid;
    static constexpr size_t kFetchTime = 32;
    // static constexpr size_t kMaxLocalFreeList = 256;
    // static constexpr size_t kMaxGlobalFreeList = 1024;

    inline constexpr size_t idx(size_t sz) {
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
    FreeList() : head_(nullptr), size_(0) {}

    void push(FreeNode *node) {
        // head insertion
        node->next = head_;
        head_ = node;
        ++size_;
    }

    FreeNode* pop() {
        if (head_ == nullptr)
            return nullptr;

        FreeNode *ret = head_;
        head_ = head_->next;
        --size_;
        return ret;
    }

    bool empty() const { return head_ == nullptr; }
    size_t size() const { return size_; }
    FreeNode* head() const { return head_; }

private:
    FreeNode* head_;
    size_t size_;
};


class GlobalPool
{
public:
    ~GlobalPool();
    FreeNode* alloc(size_t size);
    void free(FreeNode* node, size_t size);
    void free_batch(FreeNode* node, size_t size);

private:
    FreeList pool_[MemoryPool::kNumClasses];
    std::mutex mtx_;
};


class ThreadCache
{
public:
    explicit ThreadCache(GlobalPool& globalPool) : globalPool_(globalPool) {}
    ~ThreadCache();
    FreeNode* alloc(size_t size);
    void free(FreeNode* node, size_t size);

private:
    FreeList freelists_[MemoryPool::kNumClasses];
    GlobalPool& globalPool_;
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
