#include "logging.h"
#include "slice.h"

#include <limits>

namespace kvstorage {

void AppendNumberTo(std::string* str, uint64_t num) {
    str->append(std::to_string(num));
}

void AppendEscapedStringTo(std::string* str, const Slice& value) {
    str->append(value.toString());
}

std::string NumberToString(uint64_t num) {
    std::string r;
    AppendNumberTo(&r, num);
    return r;
}

std::string EscapeString(const Slice& value) {
    std::string r;
    AppendEscapedStringTo(&r, value);
    return r;
}

bool ConsumeDecimalNumber(Slice* in, uint64_t* val) {
    constexpr const uint64_t s_max_uint64 = std::numeric_limits<uint64_t>::max();
    // uint64_t 最大值的最后一位数字对应的ASCII码
    constexpr const char s_last_digit_of_max_uint64 = '0' + static_cast<char>(s_max_uint64 % 10);
    uint64_t value = 0;

    const uint8_t* start = reinterpret_cast<const uint8_t*>(in->data());
    const uint8_t* end = start + in->size();
    const uint8_t* current = start;
    for (; current != end; ++current) {
        const uint8_t ch = *current;
        if (ch < '0' || ch > '9') break;  // 非数字退出
        if (value > s_max_uint64 / 10 
            || (value == s_max_uint64 / 10 && ch > s_last_digit_of_max_uint64)) {
            // 当前值左移一位，加上当前遍历的字符大于uint64_t max，则发生溢出
            return false;
        }
        value = (value * 10) + (ch - '0');
    }
    *val = value;
    const size_t digits_consumed = current - start;
    in->removePrefix(digits_consumed);
    return digits_consumed != 0;
}

}