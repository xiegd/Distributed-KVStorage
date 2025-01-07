#ifndef D_KVSTORAGE_DATABASE_H
#define D_KVSTORAGE_DATABASE_H

// #include "export.h"
#include "slice.h"
#include "iterator.h"
#include "options.h"

namespace kvstorage {

static const int s_major_version = 1;
static const int s_minor_version = 23;

struct Options;
struct ReadOptions;
struct WriteOptions;
class WriteBatch;

//数据库特定状态的抽象句柄，快照是不可变的，可以安全地从多个线程访问
class Snapshot {
public:
    Snapshot() = default;
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

protected:
    virtual ~Snapshot();
};

struct Range {
    Range() = default;
    Range(const Slice& s, const Slice& l) : start(s), limit(l) {}

    Slice start;
    Slice limit;
};

class DataBase {
public:
    DataBase() = default;
    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;
    virtual ~DataBase() = default;

    // 打开指定名称的数据库，返回一个Status对象，表示操作是否成功, dbptr指向堆分配的DataBase对象
    static Status open(const Options& options, const std::string& name, DataBase** dbptr);

    virtual Status putKey(const WriteOptions& options, const Slice& key, const Slice& value) = 0;
    virtual Status deleteKey(const WriteOptions& options, const Slice& key) = 0;
    virtual Status write(const WriteOptions& options, WriteBatch* updates) = 0;
    // 获取指定key的value
    virtual Status getValue(const ReadOptions& options, const Slice& key, std::string& value) = 0;
    // 返回一个堆分配的迭代器，使用迭代器前需要先调用seek方法
    virtual Iterator* newIterator(const ReadOptions& options) = 0;  
    // 返回当前数据库状态的句柄，迭代器创建后，使用该句柄创建的迭代器将看到数据库的稳定快照
    virtual const Snapshot* getSnapshot() = 0;
    // 释放之前获取的快照
    virtual void releaseSnapshot(const Snapshot* snapshot) = 0;
    // 获取描述数据库的属性, property是属性名称，value是获得的属性值
    virtual bool getProperty(const Slice& property, std::string& value) = 0;
    // 获取指定范围内的键占用的空间，range是范围，n是范围数量，sizes是存储空间大小
    virtual void getApproximateSizes(const Range* range, int n, uint64_t* sizes) = 0;
    // 压缩指定范围内的键的底层存储，begin和end是范围的开始和结束键
    virtual void compactRange(const Slice* begin, const Slice* end) = 0;
};

// 销毁指定名称的数据库，options是数据库选项
Status DestroyDB(const std::string& name, const Options& options);

// 修复指定名称的数据库(如果无法打开，可能会丢失数据)，options是数据库选项
Status RepairDB(const std::string& dbname, const Options& options);

}

#endif