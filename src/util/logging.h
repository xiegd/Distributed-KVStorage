#ifndef KVSTORAGE_UTIL_LOGGING_H_
#define KVSTORAGE_UTIL_LOGGING_H_

#include <cstdint>
#include <cstdio>
#include <string>

namespace kvstorage {

class Slice;
class WritableFile;

void AppendNumberTo(std::string* str, uint64_t num);  // 将num添加到str

void AppendEscapedStringTo(std::string* str, const Slice& value);  // 将value添加到str

std::string NumberToString(uint64_t num);

std::string EscapeString(const Slice& value);  // 将value转换为string
// 将in中的数字解析为val，失败返回false，并移除in中解析出的数字部分
bool ConsumeDecimalNumber(Slice* in, uint64_t* val);  

}

#endif