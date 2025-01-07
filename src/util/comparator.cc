/*
 * 实现了一个默认比较器BytewiseComparatorImpl， 用于在skiplist或SSTable中进行key的比较
*/
#include "comparator.h"

#include "slice.h"
#include "no_destructor.h"

namespace kvstorage {

namespace {

// 默认比较器实现, 按字节比较
class BytewiseComparatorImpl : public Comparator {
public:
    BytewiseComparatorImpl() = default;
    ~BytewiseComparatorImpl() = default;

public:
    const char* name() const override { return "leveldb.BytewiseComparator"; }
    int compare(const Slice& a, const Slice& b) const override {
        return a.compare(b);
    }

    // 在start和limit之间找到一个合适的分隔符，尽可能短同时排序上位于start和limit之间
    void findShortestSeparator(std::string* start, const Slice& limit) const override {
        size_t min_length = std::min(start->size(), limit.size());
        size_t diff_index = 0;
        // 找出第一个不相同的字符的index
        while ((diff_index < min_length) && ((*start)[diff_index] == limit[diff_index])) {
            diff_index++;
        }

        if (diff_index >= min_length) {
            // 如果两个字符串完全相同， 则不缩短
        } else {
            uint8_t diff_byte = static_cast<uint8_t>((*start)[diff_index]);
            if (diff_byte < static_cast<uint8_t>(0xff) && 
                diff_byte + 1 < static_cast<uint8_t>(limit[diff_index])) {
                // <0xff 可以增加， 且增加后小于limit[diff_index]
                (*start)[diff_index]++;
                start->resize(diff_index + 1);  // 截断start，得到范围内最短的key
                assert(compare(*start, limit) < 0);
            }
        }
    }

    // 在不影响比较结果的前提下，将key转换为最小的key
    void findShortSuccessor(std::string* key) const override {
        size_t n = key->size();
        for (size_t i = 0; i < n; i++) {
            const uint8_t byte = (*key)[i];
            if (byte != static_cast<uint8_t>(0xff)) {
                (*key)[i] = byte + 1;
                key->resize(i + 1);
                return;
            }
        }
        // *key 是连续的0xff， 则不改变
    }

};
}

const Comparator* BytewiseComparator() {
    static NoDestructor<BytewiseComparatorImpl> singleton;
    return singleton.get();
}

}