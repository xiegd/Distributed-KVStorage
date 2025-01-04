/*
 *  基于 murmur hash 的变种哈希函数, 快速哈希
*/
#include "hash.h"

// fall-through：在 switch 语句中, 执行完一个 case 后, 继续执行下一个 case
// 使用自定义的 FALLTHROUGH_INTENDED 宏, 或者 [[fallthrough]] 属性，提高代码可读性

namespace kvstorage {

uint32_t Hash(const char* data, size_t n, uint32_t seed) {
    const uint32_t m = 0xc6a4a793;  // 质数乘数
    const uint32_t r = 24;  // 右移位数
    const char* limit = data + n;  // 哈希值计算的结束位置
    uint32_t h = seed ^ (n * m);  // 初始哈希值
    // 
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    // 处理剩余字节
    switch (limit - data) {
        case 3:
            h += static_cast<uint8_t>(data[2]) << 16;
            [[fallthrough]];  // [[fallthrough]] 属性用于标记 fall-through 意图
        case 2:
            h += static_cast<uint8_t>(data[1]) << 8;
            [[fallthrough]];
        case 1:
            h += static_cast<uint8_t>(data[0]);
            h *= m;
            h ^= (h >> r);
            break;
    }
    return h;
}

}  // namespace kvstorage
