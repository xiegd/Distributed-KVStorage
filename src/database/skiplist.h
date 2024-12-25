#ifndef D_KVSTORAGE_SKIPLIST_H
#define D_KVSTORAGE_SKIPLIST_H

#include <atomic>
#include <cassert>
#include <cstdlib>

#include "util/arena.h"
#include "util/random.h"

namespace kvstorage {

template <typename Key, class Comparator>
class SkipList {
private:
    struct Node;

public:
    explicit SkipList(Comparator cmp, Arena* arena);
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

public:
    void insert(const Key& key);
    bool contains(const Key& key) const;

    class Iterator {
    public:
        inline explicit Iterator(const SkipList* list);
        inline bool valid() const;
        inline const Key& key() const;
        inline void next();
        inline void prev();
        inline void seek(const Key& target);  // 移动到第一个 >= target的结点
        inline void seekToFirst();  // 移动到第一个结点
        inline void seekToLast();   // 移动到最后一个结点
    
    private:
        const SkipList* list_;
        Node* node_;
    };

private:
    inline int getMaxHeight() const;
    Node* newNode(const Key& key, int height);  // 创建一个指定高度的跳表结点
    int randomHeight();
    bool equal(const Key& a, const Key& b) const;
    bool keyIsAfterNode(const Key& key, Node* n) const;
    Node* findGreateOrEqual(const Key& key, Node** prev) const;
    Node* findLessThan(const Key& key) const;
    Node* findLast() const;

private:
    static constexpr int s_max_height_ = 12;
    Comparator const compare_;
    Arena* const arena_;
    Node* const head_;  // 跳表的头节点, 不存数据
    std::atomic<int> max_height_;  // 当前的最大高度
    Random rnd_;
};

template <typename Key, class Comparator>
struct SkipList<Key, Comparator>::Node {
public:
    Key const key;
    explicit Node(const Key& k) : key(k) {}

    Node* next(int n){
        assert(n >= 0);
        // 使用acquire语义，不允许后面的内存操作重排到load nexts_之前
        return nexts_[n].load(std::memory_order_acquire);
    }

    void setNext(int n, Node* x) {
        assert(n >= 0);
        // 使用release语义，不允许前面的内存操作重排到store nexts_之后
        nexts_[n].store(x, std::memory_order_release);
    }

    Node* noBarrierNext(int n) {
        assert(n >= 0);
        return nexts_[n].load(std::memory_order_relaxed);  // 使用relaxed语义, 只保证原子性，允许内存操作重排
    }

    void noBarrierSetNext(int n, Node* x) {
        assert(n >= 0);
        nexts_[n].store(x, std::memory_order_relaxed);
    }

private:
    std::atomic<Node*> nexts_[1];  // 跳表结点，使用柔性数组, 给所有的next分配连续的内存
};

template <typename Key, class Comparator>
SkipList<Key, Comparator>::SkipList(Comparator cmp, Arena* arena)
    : compare_(cmp), arena_(arena), head_(newNode(0, s_max_height_)), max_height_(1), rnd_(0xdeadbeef) {
    for (int i = 0; i < s_max_height_; ++i) {
        head_->setNext(i, nullptr);  // 头节点层数为s_max_height_
    }
}


template <typename Key, class Comparator>
void SkipList<Key, Comparator>::insert(const Key& key) {
    Node* prev[s_max_height_];  // 记录每一层的前驱
    Node* x = findGreateOrEqual(key, prev);  // 获取每一层的前驱

    assert(x == nullptr || !equal(key, x->key));

    int height = randomHeight();
    if (height > getMaxHeight()) {
        for (int i = getMaxHeight(); i < height; i++) {
          prev[i] = head_;  // 之前没有前驱，则前驱为head_
        }
        max_height_.store(height, std::memory_order_relaxed);
    }

    x = newNode(key, height);
    // 插入结点，更新每一层的前驱和后继
    for (int i = 0; i < height; i++) {
        x->noBarrierSetNext(i, prev[i]->noBarrierNext(i));  // 获取前驱原来的后继，设置为插入结点的后继
        prev[i]->setNext(i, x);  // 设置前驱的后继为插入结点
    }
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::contains(const Key& key) const {
    Node* x = findGreaterOrEqual(key, nullptr);
    if (x != nullptr && equal(key, x->key)) {
        return true;
    } else {
        return false;
    }
}

template <typename Key, class Comparator>
int SkipList<Key, Comparator>::getMaxHeight() const { return max_height_.load(std::memory_order_relaxed); }

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* SkipList<Key, Comparator>::newNode(const Key& key, int height) {
    char* const node_memory = arena_->allocateAligned(
        sizeof(Node) + sizeof(std::atomic<Node*>) * (height - 1));
    return new (node_memory) Node(key);  // 定位new, 配合柔性数组，创建长度为height的nexts_
}

template <typename Key, class Comparator>
int SkipList<Key, Comparator>::randomHeight() {
    static const unsigned int kBranching = 4;
    int height = 1;
    // oneIn(kBranching) 以约1/kBranching的概率返回true, 所以75%的概率 height = 1, 1/4 * 3/4的概率h = 2;
    while (height < s_max_height_ && rnd_.oneIn(kBranching)) {
      height++;
    }
    assert(height > 0);
    assert(height <= s_max_height_);
    return height;
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::equal(const Key& a, const Key& b) const {
    return (compare_(a, b) == 0);
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::keyIsAfterNode(const Key& key, Node* n) const {
    return (n != nullptr) && (compare_(n->key, key) < 0);
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::findGreateOrEqual(const Key& key, Node** prev) const {
    Node* x = head_;
    int level = getMaxHeight() - 1;
    while (true) {
        Node* next = x->next(level);
        if (keyIsAfterNode(key, next)) {
            // key在next结点之后，则更新next直到找到 >= key的结点或者nullptr
            x = next;
        } else {
            if (prev != nullptr) prev[level] = x;  // 记录前驱，if 不满足，找到了一个next->key >= key的结点
            if (level == 0) {
                return next;  // 找到了 >= key的结点
            } else {
                level--;  // 切换到下一层
            }
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::findLessThan(const Key& key) const {
    Node* x = head_;
    int level = getMaxHeight() - 1;
    while (true) {
        assert(x == head_ || compare_(x->key, key) < 0);
        Node* next = x->next(level);
        if (next == nullptr || compare_(next->key, key) >= 0) {
            // 当前遍历到的结点的next为nullptr或者next->key >= key
            if (level == 0) {
              return x;  // level == 0, 则找到了 <= key的最大结点
            } else {
              level--;  // 切换到下一层继续找
            }
        } else {
            x = next;  // 继续沿当前层往后找
        }
    }
}

template <typename Key, class Comparator>
typename SkipList<Key, Comparator>::Node* 
SkipList<Key, Comparator>::findLast() const {
    Node* x = head_;
    int level = getMaxHeight() - 1;
    while (true) {
        Node* next = x->next(level);
        if (next == nullptr) {
            if (level == 0) {
                return x;
            } else {
                level--;
            }
        } else {
            x = next;
        }
    }
}


template <typename Key, class Comparator>
SkipList<Key, Comparator>::Iterator::Iterator(const SkipList* list) {
    list_ = list;
    node_ = nullptr;
}

template <typename Key, class Comparator>
bool SkipList<Key, Comparator>::Iterator::valid() const {
    return node_ != nullptr;
}

template <typename Key, class Comparator>
const Key& SkipList<Key, Comparator>::Iterator::key() const {
    assert(valid());
    return node_->key;
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::next() {
    assert(valid());
    node_ = node_->next(0);  // 在最底层进行移动
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::prev() {
    assert(valid());
    node_ = list_->findLessThan(node_->key);
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::seek(const Key& target) {
    node_ = list_->findGreaterOrEqual(target, nullptr);
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::seekToFirst() {
    node_ = list_->head_->next(0);
}

template <typename Key, class Comparator>
void SkipList<Key, Comparator>::Iterator::seekToLast() {
    node_ = list_->findLast();
    if (node_ == list_->head_) {
        node_ = nullptr;
    }
}
    
}  // namespace kvstorage

#endif