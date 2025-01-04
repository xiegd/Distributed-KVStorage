/*
 * Status类，用于错误处理，封装了操作的结果状态和错误信息
*/
#ifndef D_KVSTORAGE_STATUS_H
#define D_KVSTORAGE_STATUS_H

#include <algorithm>
#include <string>

#include "slice.h"

namespace kvstorage {

enum class StatuCode {
    Ok = 0,
    NotFound,
    Corruption,
    NotSupported,
    InvalidArgument,
    IOError
};

class Status {
public:
    Status() noexcept : state_(nullptr) {}
    Status(const Status& rhs); 
    Status& operator=(const Status& rhs);
    Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
    Status& operator=(Status&& rhs) noexcept;
    ~Status() { delete[] state_; }


public:
    // 返回状态
    static Status success() { return Status(); }

    static Status notFound(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(StatuCode::NotFound, msg, msg2);
    }

    static Status corruption(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(StatuCode::Corruption, msg, msg2);
    }

    static Status notSupported(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(StatuCode::NotSupported, msg, msg2);
    }

    static Status invalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(StatuCode::InvalidArgument, msg, msg2);
    }

    static Status ioError(const Slice& msg, const Slice& msg2 = Slice()) {
        return Status(StatuCode::IOError, msg, msg2);
    }

    // 判断是否是某个状态
    bool ok() const { return (state_ == nullptr); }
    bool isNotFound() const { return code() == StatuCode::NotFound; }
    bool isCorruption() const { return code() == StatuCode::Corruption; }
    bool isNotSupported() const { return code() == StatuCode::NotSupported; }
    bool isInvalidArgument() const { return code() == StatuCode::InvalidArgument; }
    bool isIOError() const { return code() == StatuCode::IOError; }
    std::string toString() const;

private:
    StatuCode code() const {
        return (state_ == nullptr) ? StatuCode::Ok : static_cast<StatuCode>(state_[4]);
    }
    static const char* copyState(const char* state); 
    Status(StatuCode code, const Slice& msg, const Slice& msg2 = Slice());

private:
    /*
    ok状态的state_为nullptr, 其他状态的state_为new[]分配的内存，
    内存布局为：
    state_[0..3] == length of message
    state_[4]    == Statucode
    state_[5..]  == message
    */
    const char* state_;
};

inline Status::Status(const Status& rhs) {
    state_ = (rhs.state_ == nullptr) ? nullptr : copyState(rhs.state_);
}

inline Status& Status::operator=(const Status& rhs) {
    if (state_ != rhs.state_) {
        delete[] state_;
        state_ = (rhs.state_ == nullptr) ? nullptr : copyState(rhs.state_);
    }
    return *this;
}

inline Status& Status::operator=(Status&& rhs) noexcept {
    std::swap(state_, rhs.state_);  // swap可以自动释放rhs的资源, 处理自赋值
    return *this;
}

inline const char* Status::copyState(const char* state) {
    uint32_t size;
    std::memcpy(&size, state, sizeof(size));  // state[0..3] == length of message
    char* result = new char[size + 5];  // size + 5 == state[0..3] + code + message
    std::memcpy(result, state, size + 5);
    return result;
}

}

#endif
