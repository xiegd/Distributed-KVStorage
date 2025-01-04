#include "status.h"

#include <cstdio>

namespace kvstorage {

Status::Status(StatuCode code, const Slice& msg, const Slice& msg2) {
    assert(code != StatuCode::Ok);
    const uint32_t len1 = static_cast<uint32_t>(msg.size());
    const uint32_t len2 = static_cast<uint32_t>(msg2.size());
    const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
    // 填充状态码和错误信息
    char* result = new char[size + 5];
    std::memcpy(result, &size, sizeof(size));
    result[4] = static_cast<char>(code);
    std::memcpy(result + 5, msg.data(), len1);  // 填充msg
    if (len2) {
        result[5 + len1] = ':';
        result[6 + len1] = ' ';
        std::memcpy(result + 7 + len1, msg2.data(), len2);  // 填充msg2
    }
    state_ = result;
}

std::string Status::toString() const {
    if (state_ == nullptr) {
        return "OK";
    } else {
        char tmp[30];
        const char* type;
        switch (code()) {
            case StatuCode::Ok:
                type = "OK";
                break;
            case StatuCode::NotFound:
                type = "NotFound: ";
                break;
            case StatuCode::Corruption:
                type = "Corruption: ";
                break;
            case StatuCode::NotSupported:
                type = "Not implemented: ";
                break;
            case StatuCode::InvalidArgument:
                type = "Invalid argument: ";
                break;
            case StatuCode::IOError:
                type = "IO error: ";
                break;
            default:
                std::snprintf(tmp, sizeof(tmp), "Unknown code(%d): ", static_cast<int>(code()));
                type = tmp;
                break;
        }
        std::string result(type);
        uint32_t length;
        std::memcpy(&length, state_, sizeof(length));
        result.append(state_ + 5, length);
        return result;
    }
}

}
