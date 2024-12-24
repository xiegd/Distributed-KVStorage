#ifndef D_KVSTORAGE_SKIPLIST_H
#define D_KVSTORAGE_SKIPLIST_H

#include <atomic>
#include <cassert>
#include <cstdlib>



namespace kvstorage {

template <typename Key, class Comparator>
class SkipList {
private:
    struct Node;

public:
    explicit SkipList(Comparator cmp, Arena* arena);
};

}  // namespace kvstorage

#endif