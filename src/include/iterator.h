/*
* 迭代器接口
*/
#ifndef D_KVSTORAGE_ITERATOR_H
#define D_KVSTORAGE_ITERATOR_H

#include "slice.h"
#include "status.h"

namespace kvstorage {

class Iterator {
public:
    Iterator() = default;
    Iterator(const Iterator&) = delete;
    Iterator& operator=(const Iterator&) = delete;
    virtual ~Iterator() = default;

public:
    virtual bool valid() const = 0;
    virtual void seekToFirst() = 0;
    virtual void seekToLast() = 0;
    virtual void seek(const Slice& target) = 0;
    virtual void next() = 0;
    virtual void prev() = 0;
    virtual Slice key() const = 0;
    virtual Slice value() const = 0;
    virtual Status status() const = 0;  // 返回状态码

    using CleanupFunction = void (*)(void* arg1, void* args2);
    void registerCleanup(CleanupFunction func, void* arg1, void* arg2);

private:
    struct CleanupNode {
        bool isEmpty() const { return func == nullptr; }
        void run() {
            assert(func != nullptr);
            (*func)(arg1, arg2);
        }
        CleanupFunction func;
        void* arg1;
        void* arg2;
        CleanupNode* next;
    };
    CleanupNode cleanup_head_;
};
}


#endif