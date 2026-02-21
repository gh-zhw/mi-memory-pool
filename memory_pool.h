#pragma once

#include <stddef.h>
#include <mutex>


namespace MemoryPool
{
    static constexpr size_t kMaxSmallSize = 128;
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
    FreeNode* alloc(size_t size) {
        std::lock_guard<std::mutex> lock(mtx_);  // RAII
        FreeList& list = pool_[MemoryPool::idx(size)];
        if (list.empty()) return nullptr;
        return list.pop();
    }

    void free(FreeNode* node, size_t size) {
        std::lock_guard<std::mutex> lock(mtx_);
        pool_[MemoryPool::idx(size)].push(node);
    }

private:
    FreeList pool_[MemoryPool::kNumClasses];
    std::mutex mtx_;
};


class ThreadCache
{
public:

    FreeNode* alloc(size_t size, GlobalPool& globalPool) {
        FreeList& list = freelists_[MemoryPool::idx(size)];
        if (!list.empty()) return list.pop();

        // else allocate from GlobalPool
        for (size_t i = 0; i < MemoryPool::kFetchTime; i++) {
            if (FreeNode* node = globalPool.alloc(size)) {
                list.push(node);
            } else {
                // system alloc
                list.push(sys_alloc(size));
            }
        }

        return list.pop();
    }

    void free(FreeNode* node, size_t size) {
        freelists_[MemoryPool::idx(size)].push(node);
    }

private:
    FreeList freelists_[MemoryPool::kNumClasses];

    static FreeNode* sys_alloc(size_t size) {
        void *p = ::operator new(size);
        return static_cast<FreeNode *>(p);
    }
};
