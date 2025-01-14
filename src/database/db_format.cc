#include "db_format.h"

#include <cstdio>
#include <sstream>

#include "coding.h"
#include "logging.h"

namespace kvstorage {

std::string ParsedInternalKey::debugString() const {
    std::ostringstream ss;
    ss << '\'' << EscapeString(user_key.toString()) << "' @ " 
        << sequence << " : " << static_cast<int>(type);
    return ss.str();
}

// 将sequence和type打包为一个64位整数
static uint64_t PackSequenceAndType(uint64_t seq, ValueType t) {
    assert(seq <= s_max_sequence_number);
    assert(t == ValueType::TypeValue || t == ValueType::TypeDeletion);
    return (seq << 8) | static_cast<uint64_t>(t);
}

void AppendInternalKey(std::string* result, const ParsedInternalKey& key) {
    result->append(key.user_key.data(), key.user_key.size());
    PutFixed64(result, PackSequenceAndType(key.sequence, key.type));
}

////////////////////// InternalKeyComparator //////////////////////

const char* InternalKeyComparator::name() const {
    return "default InternalKeyComparator";
}

int InternalKeyComparator::compare(const Slice& a, const Slice& b) const {
    // 比较两个InternalKey
    // 先比较user_key(使用用户提供的Comparator)，再比较sequence，type（降序）
    int r = user_comparator_->compare(ExtractUserKey(a), ExtractUserKey(b));
    if (r == 0) {
        // 继续比较sequence和type
        const uint64_t a_num = DecodeFixed64(a.data() + a.size() - 8);
        const uint64_t b_num = DecodeFixed64(b.data() + b.size() - 8);
        if (a_num > b_num) {
            r = -1;
        } else if (a_num < b_num) {
            r = 1;
        }
    }
    return r;
}

void InternalKeyComparator::findShortestSeparator(std::string* start, const Slice& limit) const {
    // 尝试缩短user_key
    Slice user_start = ExtractUserKey(*start);  // 有接受string的构造函数，可以被视作为隐式转换方法
    Slice user_limit = ExtractUserKey(limit);
    std::string tmp(user_start.data(), user_start.size());
    // 调用用户提供的Comparator的findShortestSeparator方法, 修改tmp为最短key
    user_comparator_->findShortestSeparator(&tmp, user_limit);
    if (tmp.size() < user_start.size() && user_comparator_->compare(user_start, tmp) < 0) {
        // user_key变短，但逻辑上变大
        PutFixed64(&tmp, PackSequenceAndType(
                s_max_sequence_number, s_value_type_for_seek));
        // 确保tmp在start和limit之间
        assert(this->compare(*start, tmp) < 0);
        assert(this->compare(tmp, limit) < 0);
        start->swap(tmp);  // 更新start
    }
}

void InternalKeyComparator::findShortSuccessor(std::string* key) const {
    Slice user_key = ExtractUserKey(*key);  // 提取user_key
    std::string tmp(user_key.data(), user_key.size());
    user_comparator_->findShortSuccessor(&tmp);  // 转换为最短key
    if (tmp.size() < user_key.size() && user_comparator_->compare(user_key, tmp) < 0) {
        // user_key变短，但逻辑上变大
        PutFixed64(&tmp, PackSequenceAndType(
                s_max_sequence_number, s_value_type_for_seek));
        assert(this->compare(*key, tmp) < 0);
        key->swap(tmp);
    }
}

////////////////////// InternalFilterPolicy //////////////////////

const char* InternalFilterPolicy::name() const { return user_policy_->name(); }

void InternalFilterPolicy::createFilter(const Slice* keys, int n, std::string* dst) const {
    Slice* mkey = const_cast<Slice*>(keys);
    // 遍历所有的key, 提取user_key, 一共n个key
    for (int i = 0; i < n; ++i) {
        mkey[i] = ExtractUserKey(keys[i]);
    }
    user_policy_->createFilter(mkey, n, dst);  // 使用用户键创建过滤器，
}

bool InternalFilterPolicy::keyMayMatch(const Slice& key, const Slice& filter) const {
    return user_policy_->keyMayMatch(ExtractUserKey(key), filter);
}

////////////////////// InternalKey //////////////////////

bool InternalKey::decodeFrom(const Slice& s) {
    rep_.assign(s.data(), s.size());
    return !rep_.empty();
}

Slice InternalKey::encode() const {
    assert(!rep_.empty());
    return rep_;
}

Slice InternalKey::userKey() const { return ExtractUserKey(rep_); }

void InternalKey::setFrom(const ParsedInternalKey& p) {
    rep_.clear();
    AppendInternalKey(&rep_, p);
}

void InternalKey::clear() { rep_.clear(); }

std::string InternalKey::debugString() const {
    ParsedInternalKey parsed;
    if (ParseInternalKey(rep_, &parsed)) {
        return parsed.debugString();
    }
    std::ostringstream ss;
    ss << "(bad)" << EscapeString(rep_);
    return ss.str();
}

////////////////////// LookupKey ////////////////////////

LookupKey::LookupKey(const Slice& user_key, SequenceNumber sequence) {
    size_t usize = user_key.size();
    size_t needed = usize + 13;
    char* dst;
    if (needed <= sizeof(space_)) {
        dst = space_;
    } else {
        dst = new char[needed];
    }
    start_ = dst;
    dst = EncodeVarint32(dst, usize + 8);
    key_start_ = dst;
    std::memcpy(dst, user_key.data(), usize);
    dst += usize;
    EncodeFixed64(dst, PackSequenceAndType(sequence, s_value_type_for_seek));
    dst += 8;
    end_ = dst;
}

}
