/*
 *  编码和解码函数
 *  数据按字节编码到string/char*中，而解码时解码一部分，更新指针位置到剩余数据；
 *  Fixed: 固定长度编码, 使用4bytes/8bytes将一个u32或u64的无符号数编码到每一个字节;
 *  Varint: 变长编码, 每个字节后7位存数据，最高位作为标志位，为1表示字符未结束, 从而节省存储空间;
 *  由于每个字节只有7位存储数据，所以u32最多需要5bytes编码，u64最多需要10bytes编码;
 *
 *  Put: 将value编码然后添加到dst中
 *  Get: 从dst中解码出value
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
const char* GetVarint32PtrFallback(const char* p, const char* limit, uint32_t* value);

const char* GetVarint32Ptr(const char* p, const char* limit, uint32_t* value);
const char* GetVarint64Ptr(const char* p, const char* limit, uint64_t* value);

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

}

#endif