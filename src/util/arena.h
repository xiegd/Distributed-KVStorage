/*
* 内存分配器, arena
* 用于临时对象的分配
*/
#ifndef D_KVSTORAGE_ARENA_H
#define D_KVSTORAGE_ARENA_H

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace kvstorage {

class Arena {
public:
    Arena();
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    ~Arena();

public:
    char* allocate(size_t bytes);
    char* allocateAligned(size_t bytes);
    size_t memoryUsage() const;

private:
    char* allocateFallback(size_t bytes);  // 当前内存块剩余空间不足时，分配请求的内存
    char* allocateNewBlock(size_t block_bytes);  // 创建新的内存块

private:
    char* alloc_ptr_;  // 当前正在进行分配的内存块的当前指针位置
    size_t alloc_bytes_remaining_;  // 当前正在进行分配的内存块的剩余空间
    std::vector<char*> blocks_;  // 当前已经分配的所有内存块的地址
    std::atomic<size_t> memory_usage_;  // 当前累计分配的内存
};

// 显式内联，定义必须是在头文件中，此外还会进行隐式内联 
inline char* Arena::allocate(size_t bytes) {
    assert(bytes > 0);  // 不允许分配0 bytes
    if (bytes <= alloc_bytes_remaining_) {
        // 如果剩余空间足够，直接分配
        char* res = alloc_ptr_;
        alloc_ptr_ += bytes;
        alloc_bytes_remaining_ -= bytes;
        return res;
    }
    // 如果剩余空间不足，则分配新的内存块
    return allocateFallback(bytes);
}

}  // namespace kvstorage
#endif