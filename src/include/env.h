/*
 * 为不同的os统一接口
*/
#ifndef D_KVSTORAGE_ENV_H
#define D_KVSTORAGE_ENV_H

#include <cstdarg>

#include "status.h"

namespace kvstorage {

class FileLock;
class Logger;
class RandomAccessFile;
class SequentialFile;
class Slice;
class WritableFile;

class Env {
public:
    Env() = default;
    Env(const Env&) = delete;
    Env& operator=(const Env&) = delete;
    virtual ~Env() = default;

public:
    static Env* defaultEnv();  // 返回合适的默认环境
    // 创建一个对象，用于按顺序读取指定文件, 文件指针存在res中
    virtual Status newSequentialFile(const std::string& fname, SequentialFile** res) = 0;  
    // 创建一个对象，用于随机访问指定文件, 文件指针存在res中
    virtual Status newRandomAccessFile(const std::string& fname, RandomAccessFile** res) = 0;
    // 创建一个对象，用于写入指定名称的新文件, 删除已存在的同名文件, 文件指针存在res中
    virtual Status newWritableFile(const std::string& fname, WritableFile** res) = 0;
    // 创建一个对象，用于追加写入指定文件, 如果文件不存在，则创建一个新文件
    virtual Status newAppendableFile(const std::string& fname, WritableFile** res) = 0;  // 还未实现，
    virtual bool fileExists(const std::string& fname) = 0;  // 检查指定文件是否存在
    // 获取指定目录下的所有子目录名和文件名，存到res中
    virtual Status getChildren(const std::string& dir, std::vector<std::string>* res) = 0;
    virtual Status removeFile(const std::string& fname) = 0;  // 默认实现调用deleteFile, 后续需要重写， 最终移除默认实现
    virtual Status deleteFile(const std::string& fname);  // 需要重写

    virtual Status createDir(const std::string& dirname) = 0;
    virtual Status removeDir(const std::string& dirname) = 0;  // 和removeFile类似， 需要重写
    virtual Status deleteDir(const std::string& dirname);  // 也需要重写，最后移除默认实现

    virtual Status getFileSize(const std::string& fname, uint64_t* file_size) = 0;
    virtual Status renameFile(const std::string& src, const std::string& target) = 0;
    virtual Status lockFile(const std::string& fname, FileLock** lock) = 0;  // 锁定指定的文件, 防止多个进程同时访问
    virtual Status unlockFile(FileLock* lock) = 0;  // 释放使用lockFile获取的锁
    virtual void schedule(void(*function)(void* arg), void* arg) = 0;  // 安排在后台线程执行, 可能并发
    virtual void startThread(void(*function)(void* arg), void* arg) = 0;  // 启动一个线程，执行function(arg), 返回时销毁线程
    virtual Status getTestDirectory(std::string* path) = 0;  // 返回一个临时目录，用于测试
    virtual Status newLogger(const std::string& fname, Logger** res) = 0;  // 创建返回一个日志文件存消息
    virtual uint64_t nowTimeMicros() = 0;  // 返回自某个时间以来的ms
    virtual void sleepForMicroseconds(int micros) = 0;  // 线程延迟指定时间
};

// 顺序文件的抽象
class SequentialFile {
public:
    SequentialFile() = default;
    SequentialFile(const SequentialFile&) = delete;
    SequentialFile& operator=(const SequentialFile&) = delete;
    virtual ~SequentialFile() = default;
    // 需要外部同步
    // 从文件中读取n字节，scratch用于临时存储数据，防止意外修改res指向的内存区域
    virtual Status read(size_t n, Slice* res, char* scratch) = 0;  
    virtual Status skip(uint64_t n) = 0;  // 跳过n字节
};

class RandomAccessFile {
public:
    RandomAccessFile() = default;
    RandomAccessFile(const RandomAccessFile&) = delete;
    RandomAccessFile& operator=(const RandomAccessFile&) = delete;
    virtual ~RandomAccessFile() = default;

    virtual Status read(uint64_t offset, size_t n, Slice* res, char* scrath) = 0;  // 从文件中读取n字节, 线程安全
};

class WritableFile {
public:
    WritableFile() = default;
    WritableFile(const WritableFile&) = delete;
    WritableFile& operator=(const WritableFile&) = delete;
    virtual ~WritableFile() = default;

    virtual Status append(const Slice& data) = 0;  // 追加写入数据
    virtual Status close() = 0;  // 关闭文件
    virtual Status flush() = 0;  // 刷新缓冲区
    virtual Status sync() = 0;  // 同步数据到磁盘
};

class Logger {
public:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    virtual ~Logger() = default;

    virtual void logv(const char* format, std::va_list ap) = 0;  // 以指定格式向日志中写入记录
};

class FileLock {
public:
    FileLock() = default;
    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;
    virtual ~FileLock() = default;
};

// 将指定数据按照指定格式写入日志
void Log(Logger* info_log, const char* format, ...)
#if defined(__GNUC__) || defined(__clang__)
    // 编译时检查格式化串和参数是否匹配
    __attribute__((__format__(__printf__, 2, 3)))  // 2: 格式化串, 3: 可变参数列表
#endif
;

// 将指定数据写入指定文件
Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname);

// 从指定文件中读取数据
Status ReadFileToString(Env* env, const std::string& fname, std::string* data);

// 一个Env的包装类, 将所有方法调用转发给另一个Env
class EnvWrapper : public Env {
public:
    explicit EnvWrapper(Env* t) : target_(t) {}
    virtual ~EnvWrapper() {};

    Env* target() const { return target_; }

    Status newSequentialFile(const std::string& f, SequentialFile** r) override {
        return target_->newSequentialFile(f, r);
    }

    Status newRandomAccessFile(const std::string& f, RandomAccessFile** r) override {
        return target_->newRandomAccessFile(f, r);
    }

    Status newWritableFile(const std::string& f, WritableFile** r) override {
        return target_->newWritableFile(f, r);
    }

    Status newAppendableFile(const std::string& f, WritableFile** r) override {
        return target_->newAppendableFile(f, r);
    }

    bool fileExists(const std::string& f) override { return target_->fileExists(f); }

    Status getChildren(const std::string& dir, std::vector<std::string>* r) override {
        return target_->getChildren(dir, r);
    }

    Status removeFile(const std::string& f) override { return target_->removeFile(f); }
    Status createDir(const std::string& d) override { return target_->createDir(d); }
    Status removeDir(const std::string& d) override { return target_->removeDir(d); }
    Status getFileSize(const std::string& f, uint64_t* s) override { return target_->getFileSize(f, s); }
    Status renameFile(const std::string& s, const std::string& t) override { return target_->renameFile(s, t); }
    Status lockFile(const std::string& f, FileLock** l) override { return target_->lockFile(f, l); }
    Status unlockFile(FileLock* l) override { return target_->unlockFile(l); }
    void schedule(void (*f)(void*), void* a) override { return target_->schedule(f, a); }
    void startThread(void (*f)(void*), void* a) override { return target_->startThread(f, a); }
    Status getTestDirectory(std::string* path) override { return target_->getTestDirectory(path); }
    Status newLogger(const std::string& fname, Logger** result) override { return target_->newLogger(fname, result); }
    uint64_t nowTimeMicros() override { return target_->nowTimeMicros(); }
    void sleepForMicroseconds(int micros) override { return target_->sleepForMicroseconds(micros); }

private:
    Env* target_;
};

}

// 处理win下DeleteFile的命名冲突
#if defined(_WIN32) && defined(LEVELDB_DELETEFILE_UNDEFINED)
#if defined(UNICODE)
#define DeleteFile DeleteFileW
#else
#define DeleteFile DeleteFileA
#endif  // defined(UNICODE)
#endif  // defined(_WIN32) && defined(LEVELDB_DELETEFILE_UNDEFINED)

#endif