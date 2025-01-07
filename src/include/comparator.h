/*
 * Comparator 比较器的基类， 定义了比较器的基本接口， 用于在skiplist或SSTable中进行key的比较
*/
#ifndef D_KVSTORAGE_COMPARATOR_H
#define D_KVSTORAGE_COMPARATOR_H

#include "string"

namespace kvstorage {

class Slice;

class Comparator {
public:
    Comparator() = default;
    virtual ~Comparator() = default;
    // a < b, < 0; a == b, == 0; a > b, > 0
    virtual int compare(const Slice& a, const Slice& b) const = 0;
    // 返回Comparator的名称， 用于检查comparator是否匹配
    virtual const char* name() const = 0;
    // 在不影响比较结果的前提下，在start和limit之间找到最短的key(即在不影响排序结果的范围中找)， 用于优化存储和检索效率
    virtual void findShortestSeparator(std::string* start, const Slice& limit) const = 0;
    // 在不影响比较结果的前提下，将key转换为最小的key
    virtual void findShortSuccessor(std::string* key) const = 0;
};

const Comparator* BytewiseComparator();

}

#endif