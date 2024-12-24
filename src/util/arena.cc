#include "arena.h"

namespace kvstorage {

static const int kBlockSize = 4096;

Arena::Arena() : alloc_ptr_(nullptr), alloc_bytes_remaining_(0), memory_usage_(0) {}

Arena::~Arena() {
    for (size_t i = 0; i < blocks_.size(); ++i) {
        delete[] blocks_[i];
    }
}

size_t Arena::memoryUsage() const { return memory_usage_.load(std::memory_order_relaxed); }

char* Arena::allocateFallback(size_t bytes) {
    if (bytes > kBlockSize / 4) {
        // 请求的内存大于1/4个block，则直接分配新的block, 避免内存碎片
        char* result = allocateNewBlock(bytes);
        return result;
    }

    alloc_ptr_ = allocateNewBlock(kBlockSize);
    alloc_bytes_remaining_ = kBlockSize;

    char* result = alloc_ptr_;
    alloc_ptr_ += bytes;
    alloc_bytes_remaining_ -= bytes;
    return result;
}

char* Arena::allocateAligned(size_t bytes) {
    const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;  // 确定系统指针大小, 确定对齐所需的最小字节数
    static_assert((align & (align - 1)) == 0, "Pointer size should be a power of 2");  // 编译时检查align
    size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align - 1);
    size_t slop = (current_mod == 0 ? 0 : align - current_mod);  // 计算需要对齐的字节数
    size_t needed = bytes + slop;  // 对齐后需要分配的字节数
    char* res;
    if (needed <= alloc_bytes_remaining_) {
        res = alloc_ptr_ + slop;
        alloc_ptr_ += needed;
        alloc_bytes_remaining_ -= needed;
    } else {
        res = allocateFallback(bytes);
    }
    // uintptr_t 存储指针值的整数类型，自适应系统位数。运行时，检查地址res是否对齐(char* 按1字节对齐，需要检查)
    assert((reinterpret_cast<uintptr_t>(res) & (align - 1)) == 0);  
    return res;
}

char* Arena::allocateNewBlock(size_t block_bytes) {
    char* res = new char[block_bytes];
    blocks_.push_back(res);
    memory_usage_.fetch_add(block_bytes + sizeof(char*), std::memory_order_relaxed);
    return res;
}

}  // namespace kvstorage