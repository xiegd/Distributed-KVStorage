/*
 * Slice类，字符数组的封装
*/
#ifndef D_KVSTORAGE_SLICE_H
#define D_KVSTORAGE_SLICE_H

#include <cstddef>
#include <cassert>
#include <cstring>
#include <string>

namespace kvstorage {

class Slice {
public:
    Slice() : data_(""), size_(0) {}
    Slice(const char* s, size_t len) : data_(s), size_(len) {}
    Slice(const char* s) : data_(s), size_(strlen(s)) {}
    Slice(const std::string& s) : data_(s.data()), size_(s.size()) {}

    Slice(const Slice&) = default;
    Slice& operator=(const Slice&) = default;

public:
    const char* data() const { return data_; }
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
    const char* begin() const { return data_; }
    const char* end() const { return data_ + size_; }

    bool startsWith(const Slice& x) const {
        return ((size_ >= x.size_) && (memcmp(data_, x.data_, x.size_)) == 0);
    }

    char operator[](size_t idx) const {
        assert(idx < size_);
        return data_[idx];
    }

    void clear() {
        data_ = "";
        size_ = 0;
    }

    void removePrefix(size_t n) {
        assert(n <= size_);
        data_ += n;
        size_ -= n;
    }
    
    std::string toString() const { return std::string(data_, size_); }
    int compare(const Slice& b) const {
        const size_t min_len = (size_ < b.size_) ? size_ : b.size_;
        int res = memcmp(data_, b.data_, min_len);  // 逐字节比较
        if (res == 0) {
            if (size_ < b.size_)
                res = -1;
            else if (size_ > b.size_)
                res = 1;
        }
        return res;
    }

private:
    const char* data_;
    size_t size_;
};

inline bool operator==(const Slice& lhs, const Slice& rhs) {
    return ((lhs.size() == rhs.size()) && (memcmp(lhs.data(), rhs.data(), lhs.size()) == 0));
}

inline bool operator!=(const Slice& lhs, const Slice& rhs) { return !(lhs == rhs); }

}

#endif