#ifndef D_KVSTORAGE_DB_FORMAT_H
#define D_KVSTORAGE_DB_FORMAT_H

#include "cstddef"
#include "cstdint"
#include "string"

#include "slice.h"
#include "comparator.h"
#include "filter_policy.h"
#include "coding.h"

namespace kvstorage {

// 配置常量
namespace config {
    static const int s_num_levels = 7;  
    static const int s_l0_compaction_trigger = 4;  // 当达到这么多文件时，开始 Level-0 压缩。
    static const int s_l0_slowdown_writes_trigger = 8;  // 减慢写入速度时的文件数量
    static const int s_l0_stop_writes_trigger = 12;  // 停止写入时的文件数量
    static const int s_max_mem_compact_level = 2;  // 新的memtable不重叠时，可以被推送到的最大层级
    static const int s_read_bytes_period = 1048576;  // 迭代过程中读取数据时，采样之间的大致字节间隔。
}

class InternalKey;

// 标记键值对的状态， 删除一个键时，不会立即物理删除而是写入一个TypeDeletion的键值对
// 墓碑标记，TypeValue表示正常的键值对
enum class ValueType {
    TypeDeletion = 0x0,
    TypeValue = 0x1
};

static const ValueType s_value_type_for_seek = ValueType::TypeValue;

using SequenceNumber = uint64_t;

// 序列号和值类型一起存储，所以序列号最大值为 1 << 56 - 1
static const SequenceNumber s_max_sequence_number = ((0x1ull << 56) - 1);

struct ParsedInternalKey {
    ParsedInternalKey() {}
    ParsedInternalKey(const Slice& u, const SequenceNumber& seq, ValueType t) 
        : user_key(u), sequence(seq), type(t) {}
    std::string debugString() const;

    Slice user_key;
    SequenceNumber sequence;
    ValueType type;
};

// 返回内部键的编码长度
inline size_t InternalKeyEncodingLength(const ParsedInternalKey& key) {
    return key.user_key.size() + 8;
}

// 将ParsedInternalKey编码为字符串，追加到result中
void AppendInternalKey(std::string* result, const ParsedInternalKey& key);

// 从Slice类型的internal_key中解析出user_key, sequence, type保存为一个ParsedInternalKey
bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result);

inline Slice ExtractUserKey(const Slice& internal_key) {
    assert(internal_key.size() >= 8);
    // 去除8bytes的sequence和type，剩余的就是变长的user_key
    return Slice(internal_key.data(), internal_key.size() - 8);
}

class InternalKeyComparator : public Comparator {
public:
    explicit InternalKeyComparator(const Comparator* c) : user_comparator_(c) {}
    const char* name() const override;
    int compare(const Slice& a, const Slice& b) const override;
    int compare(const InternalKey& a, const InternalKey& b) const;  // 未实现
    void findShortestSeparator(std::string* start, const Slice& limit) const override;
    void findShortSuccessor(std::string* key) const override;
    const Comparator* userComparator() const { return user_comparator_; }

private:
    const Comparator* user_comparator_;  // 提供用户比较器, 默认按字符序比较
};

class InternalFilterPolicy : public FilterPolicy {
public:
    explicit InternalFilterPolicy(const FilterPolicy* p) : user_policy_(p) {}
    const char* name() const override;
    void createFilter(const Slice* keys, int n, std::string* dst) const override;
    bool keyMayMatch(const Slice& key, const Slice& filter) const override;  // 检查键是否可能在过滤器中

private:
    const FilterPolicy* const user_policy_;
};

class InternalKey {
public:
    InternalKey() {}
    InternalKey(const Slice& user_key, SequenceNumber s, ValueType t) {
        AppendInternalKey(&rep_, ParsedInternalKey(user_key, s, t));
    }
    bool decodeFrom(const Slice& s);
    Slice encode() const;
    Slice userKey() const;
    void setFrom(const ParsedInternalKey& p);
    void clear();
    std::string debugString() const;

private:
    std::string rep_;
};

inline int InternalKeyComparator::compare(const InternalKey& a, const InternalKey& b) const {
    return compare(a.encode(), b.encode());
}

inline bool ParseInternalKey(const Slice& internal_key, ParsedInternalKey* result) {
    const size_t n = internal_key.size();  // 获取键的总长度
    if (n < 8) return false;
    uint64_t num = DecodeFixed64(internal_key.data() + n - 8);  // 跳过用户键, 解码序列号和值类型
    uint8_t c = num & 0xff;
    result->sequence = num >> 8;  // 右移8位，去除valuetype
    result->type = static_cast<ValueType>(c);
    result->user_key = Slice(internal_key.data(), n - 8);  // 除去最后8字节，剩下的就是user_key
    return (c <= static_cast<uint8_t>(ValueType::TypeValue));
}

// DBImpl::Get()的辅助类
class LookupKey {
public:
    // 使用指定的序列号初始化此对象，用于在快照中查找 user_key
    LookupKey(const Slice& user_key, SequenceNumber sequence);
    LookupKey(const LookupKey&) = delete;
    LookupKey& operator=(const LookupKey&) = delete;
    ~LookupKey() { 
        if (start_ != space_) delete[] start_;  // 如果没有使用space_, 则在堆上分配了内存，需要释放
    }

    Slice memtableKey() const { return Slice(start_, end_ - start_); }
    Slice internalKey() const { return Slice(key_start_, end_ - key_start_); }
    Slice userKey() const { return Slice(key_start_, end_ - key_start_ - 8); }

private:
    const char* start_;
    const char* key_start_;
    const char* end_;
    char space_[200];  // 避免对短键进行内存分配
};

}

#endif