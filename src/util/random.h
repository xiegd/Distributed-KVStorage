/*
 * 线性同余法(LCG, Linear Congruential Generator)，生成伪随机数
*/
#ifndef D_KVSTORAGE_RANDOM_H
#define D_KVSTORAGE_RANDOM_H

#include <cstdint>

namespace kvstorage {

class Random {
private:
    uint32_t seed_;

public:
    explicit Random(uint32_t s) : seed_(s & 0x7fffffffu) {
        if (seed_ == 0 || seed_ == 2147483647L) {
            seed_ = 1;
        }
    }

    uint32_t next() {
        static const uint32_t M = 2147483647L;  // 2^31-1, 模数，
        static const uint64_t A = 16807;        // 乘数，特别选择的素数，bits 14, 8, 7, 5, 2, 1, 0
        // 模数和乘数的选择是为了确保随机数序列的均匀分布和周期性
        // 计算seed_ = (seed_ * A) % M, 更新seed_
        uint64_t product = seed_ * A;
        seed_ = static_cast<uint32_t>((product >> 31) + (product & M));
        if (seed_ > M) {
            seed_ -= M;  // 处理溢出，保证seed_在[1, M-1]之间
        }
        return seed_;
    }

    // 返回一个均匀分布的值在[0, n-1]之间, n > 0
    uint32_t uniform(int n) { return next() % n; }
    // 以约1/n的概率返回true, n > 0
    bool oneIn(int n) { return (next() % n) == 0; }
    // 先从[0, max_log]中均匀分布地选择一个数base，然后从[0, 2^base-1]中选择一个数, 倾向于返回较小的数
    uint32_t skewed(int max_log) { return uniform( 1 << uniform(max_log + 1)); }

};

}

#endif