#include "./memory_pool.h"


/*
    GlobalPool
 */
FreeNode* GlobalPool::alloc(size_t size) {
    std::lock_guard<std::mutex> lock(mtx_);  // RAII
    FreeList& list = pool_[MemoryPool::idx(size)];
    if (list.empty()) return nullptr;
    return list.pop();
}

void GlobalPool::free(FreeNode* node, size_t size) {
    std::lock_guard<std::mutex> lock(mtx_);
    pool_[MemoryPool::idx(size)].push(node);
}


/*
    ThreadCache
 */
FreeNode* ThreadCache::alloc(size_t size, GlobalPool& globalPool) {
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

void ThreadCache::free(FreeNode* node, size_t size) {
    freelists_[MemoryPool::idx(size)].push(node);
}

FreeNode* ThreadCache::sys_alloc(size_t size) {
    void *p = ::operator new(size);
    return static_cast<FreeNode *>(p);
}


/*
    MiMemoryPool
 */
void* MiMemoryPool::alloc(size_t size) {
    // align to kClassGrid
    size = (MemoryPool::idx(size) + 1) * MemoryPool::kClassGrid;

    // allocate from system if size is too large
    if (size > MemoryPool::kMaxSmallSize) {
        return ::operator new(size);
    }
    FreeNode* ptr = tls_cache().alloc(size, globalPool_);

    return static_cast<void*>(ptr);
}

void MiMemoryPool::free(void* ptr, size_t size) {
    if (ptr == nullptr) return;

    size = (MemoryPool::idx(size) + 1) * MemoryPool::kClassGrid;
    if (size > MemoryPool::kMaxSmallSize) {
        ::operator delete(ptr);
        return;
    }

    tls_cache().free(static_cast<FreeNode*>(ptr), size);
}

ThreadCache& MiMemoryPool::tls_cache() {
    thread_local ThreadCache cache;
    return cache;
}
