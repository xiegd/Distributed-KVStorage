#include "coding.h"
#include "slice.h"

#include <string>

namespace kvstorage {

// 将4
char* EncodeVarint32(char* dst, uint32_t value) {
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
    static const int B = 128;  // 用来设置字节最高位为1，表示后面还有字符
    if (value < (1 << 7)) {
        // value < 2^7, 使用1byte
        *(ptr++) = value;
    } else if (value < (1 << 14)) {
        // value < 2^14, 使用2byte
        *(ptr++) = value | B;
        *(ptr++) = value >> 7;
    } else if (value < (1 << 21)) {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = value >> 14;
    } else if (value < (1 << 28)) {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = (value >> 14) | B;
        *(ptr++) = value >> 21;
    } else {
        *(ptr++) = value | B;
        *(ptr++) = (value >> 7) | B;
        *(ptr++) = (value >> 14) | B;
        *(ptr++) = (value >> 21) | B;
        *(ptr++) = value >> 28;
    }
    return reinterpret_cast<char*>(ptr);
}

char* EncodeVarint64(char* dst, uint64_t value) {
    static const int B = 128;
    uint8_t* ptr = reinterpret_cast<uint8_t*>(dst);
    while (value >= B) {
        *(ptr++) = value | B;
        value >>= 7;
    }
    *(ptr++) = static_cast<uint8_t>(value);
    return reinterpret_cast<char*>(ptr);
}

// 将4字节的value按字节编码到buffer数组中的不同位置, 然后追加到dst中
void PutFixed32(std::string* dst, uint32_t value) {
    char buf[sizeof(value)];
    EncodeFixed32(buf, value);
    dst->append(buf, sizeof(buf));
}

// 将8字节的value按字节编码到buffer数组中的不同位置, 然后追加到dst中
void PutFixed64(std::string* dst, uint64_t value) {
    char buf[sizeof(value)];
    EncodeFixed64(buf, value);
    dst->append(buf, sizeof(buf));
}

void PutVarint32(std::string* dst, uint32_t value) {
    char buf[5];
    char* ptr = EncodeVarint32(buf, value);
    dst->append(buf, ptr - buf);
}

void PutVarint64(std::string* dst, uint64_t value) {
    char buf[10];
    char* ptr = EncodeVarint64(buf, value);
    dst->append(buf, ptr - buf);
}

// 将字符串value的长度使用变长编码进行编码，然后追加到dst中
void PutLengthPrefixedSlice(std::string* dst, const Slice& value) {
    PutVarint32(dst, value.size());
    dst->append(value.data(), value.size());
}

// 解码变长整数，p中的一个char存了一个u8，从低位解码到高位, 返回解码后剩余的p
const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value) {
    uint32_t result = 0;
    for (uint32_t shift = 0; shift <= 28 && p < limit; shift += 7) {
        uint32_t byte = *(reinterpret_cast<const uint8_t*>(p));
        p++;
        if (byte & 128) {
            // 最高位为1, 表示仍属于同一个部分
            result |= ((byte & 127) << shift);
        } else {
            // 最高位为0, 表示已经到达部分末尾
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<const char*>(p);
        }
    }
    return nullptr;  // 处理输入数据问题导致的解码失败
}

bool GetVarint32(Slice* input, uint32_t* value) {
    const char* p = input->data();  // 字符串的首地址
    const char* limit = p + input->size();  // 字符串的尾地址的后一个位置
    const char* q = GetVarint32Ptr(p, limit, value);
    if (q == nullptr) {
        return false;  // 解码失败 
    } else {
        *input = Slice(q, limit - q);  // 解码成功，更新input指针位置
        return true;
    }
}

bool GetVarint64(Slice* input, uint64_t* value) {
    const char* p = input->data();
    const char* limit = p + input->size();
    const char* q = GetVarint64Ptr(p, limit, value);
    if (q == nullptr) {
        return false;
    } else {
        *input = Slice(q, limit - q);
        return true;
    }
}

// 解码长度前缀, 获取相应长度的数据保存到result中，并更新input指针位置
bool GetLengthPrefixedSlice(Slice* input, Slice* result) {
    uint32_t len;
    if (GetVarint32(input, &len) && input->size() >= len) {
        // 解码成功，且input剩余长度大于len
        *result = Slice(input->data(), len);  // 获取对应长度的数据
        input->removePrefix(len);
        return true;
    } else {
        return false;
    }
}

const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value) {
    if (p < limit) {
        uint32_t result = *(reinterpret_cast<const uint8_t*>(p));
        if ((result & 128) == 0) {
            // 如果p有效，且最高位为0，则说明解码的数据<2^7直接解码返回, 优化性能
            *value = result;
            return p + 1;
        }
    }
    return GetVarint32PtrFallback(p, limit, value);
}

// 解码u64，优化收益小，不优化了
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value) {
    uint64_t result = 0;
    for (uint32_t shift = 0; shift <= 63 && p < limit; shift += 7) {
        uint64_t byte = *(reinterpret_cast<const uint8_t*>(p));
        p++;
        if (byte & 128) {
            result |= ((byte & 127) << shift);
        } else {
            result |= (byte << shift);
            *value = result;
            return reinterpret_cast<const char*>(p);
        }
    }
    return nullptr;
}

int VarintLength(uint64_t v) {
    int len = 1;
    while (v >= 128) {
        v >>= 7;
        len++;
    }
    return len;
}

}