/*
 *  编码和解码函数
 *  Fixed: 固定长度编码
 *  Varint: 变长编码
 *  LengthPrefixedSlice: 长度前缀编码
*/
#ifndef KVSTORAGE_UTIL_CODING_H_
#define KVSTORAGE_UTIL_CODING_H_

#include <cstdint>
#include <cstring>
#include <string>

#include "slice.h"

namespace kvstorage {

void PutFixed32(std::string* dst, uint32_t value);
void PutFixed64(std::string* dst, uint64_t value);
void PutVarint32(std::string* dst, uint32_t value);
void PutVarint64(std::string* dst, uint64_t value);
void PutLengthPrefixedSlice(std::string* dst, const Slice& value);

bool GetVarint32(Slice* input, uint32_t* value);
bool GetVarint64(Slice* input, uint64_t* value);
bool GetLengthPrefixedSlice(Slice* input, Slice* result);

const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* v);
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* v);

int VarintLength(uint64_t v);

char* EncodeVarint32(char* dst, uint32_t value);
char* EncodeVarint64(char* dst, uint64_t value);

// 将4字节的value按字节编码到buffer数组中的不同位置
inline void EncodeFixed32(char* dst, uint32_t value) {
    uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

    buffer[0] = static_cast<uint8_t>(value);  // 强制转换时发生高位截断
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
}

inline void EncodeFixed64(char* dst, uint64_t value) {
    uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

    buffer[0] = static_cast<uint8_t>(value);
    buffer[1] = static_cast<uint8_t>(value >> 8);
    buffer[2] = static_cast<uint8_t>(value >> 16);
    buffer[3] = static_cast<uint8_t>(value >> 24);
    buffer[4] = static_cast<uint8_t>(value >> 32);
    buffer[5] = static_cast<uint8_t>(value >> 40);
    buffer[6] = static_cast<uint8_t>(value >> 48);
    buffer[7] = static_cast<uint8_t>(value >> 56);
}

inline uint32_t DecodeFixed32(const char* ptr) {
    const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

    return (static_cast<uint32_t>(buffer[0]))
           | (static_cast<uint32_t>(buffer[1]) << 8)
           | (static_cast<uint32_t>(buffer[2]) << 16)
           | (static_cast<uint32_t>(buffer[3]) << 24);
}

inline uint64_t DecodeFixed64(const char* ptr) {
    const uint8_t* const buffer = reinterpret_cast<const uint8_t*>(ptr);

    return (static_cast<uint64_t>(buffer[0]))
           | (static_cast<uint64_t>(buffer[1]) << 8)
           | (static_cast<uint64_t>(buffer[2]) << 16)
           | (static_cast<uint64_t>(buffer[3]) << 24)
           | (static_cast<uint64_t>(buffer[4]) << 32)
           | (static_cast<uint64_t>(buffer[5]) << 40)
           | (static_cast<uint64_t>(buffer[6]) << 48)
           | (static_cast<uint64_t>(buffer[7]) << 56);
}

const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value);

inline const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value) {
    if (p < limit) {
        uint32_t result = *(reinterpret_cast<const uint8_t*>(p));
        if ((result & 128) == 0) {
            *value = result;
            return p + 1;
        }
    }
    return GetVarint32PtrFallback(p, limit, value);
}
}

#endif