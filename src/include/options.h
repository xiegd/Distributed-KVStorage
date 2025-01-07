/*
 * 数据库选项和配置
*/
#ifndef D_KVSTORAGE_OPTIONS_H
#define D_KVSTORAGE_OPTIONS_H

#include <cstddef>

namespace kvstorage {

class Cache;
class Comparator;
class Env;
class FilterPolicy;
class Logger;
class Snapshot;

// 数据库内容存储再一组块中，每个块包含一系列的键值对，每个块存储到文件前可能会被压缩
enum class CompressionType {
    // 枚举压缩类型
    NoCompression = 0x0,
    SnappyCompression = 0x1,
    ZstdCompression = 0x2,
};

struct Options {
    Options();

    // 影响行为的参数
    const Comparator* comparator;
    bool create_if_missing = false;  // 数据库不存在时是否创建
    bool error_if_exists = false;  // 数据库已存在时是否报错
    bool paranoid_checks = false;  // 是否进行严格检查, 严格检查会在出现任何错误时停止
    Env* env;  // 用于与环境交互, 例如读写文件, 调度后台工作等
    Logger* info_log = nullptr;  // 用于记录数据库生成的任何内部进度/错误信息
    // 影响性能的参数
    size_t write_buffer_size = 4 * 1024 * 1024;  // 写缓冲区大小
    int max_open_files = 1000;  // db可以打开的数据库文件数量
    Cache* block_cache = nullptr;  // 块缓存, 为空则使用默认创建的8MB缓存
    size_t block_size = 4 * 1024;  // 对应的未压缩数据的块的近似大小
    int block_restart_interval = 16;  // 重启点的间隔
    size_t max_file_size = 2 * 1024 * 1024;  // 数据库文件的最大大小
    CompressionType compression = CompressionType::SnappyCompression;  // 使用的压缩算法
    int zstd_compression_level = 1;  // zstd压缩级别
    bool reuse_logs = false;  // 是否重用MANIFEST和日志文件, 还未支持
    const FilterPolicy* filter_policy = nullptr;  // 过滤策略
};

// 控制读取操作的选项
struct ReadOptions {
    ReadOptions() = default;

    bool verify_checksums = false;  // 是否对读取的数据进行校验和检查
    bool fill_cache = true;  // 是否将迭代读取的数据缓存到内存
    const Snapshot* snapshot = nullptr;  // 若不为空，则从给定的快照进行读取 
};

// 控制写入操作的选项
struct WriteOptions {
    WriteOptions() = default;

    // 控制写入操作的持久性和可靠性, 决定在写入操作被认为完成前，数据是否需要从操作系统缓冲区刷新到磁盘
    // true: 写入操作完成后，数据会从操作系统缓冲区刷新到磁盘
    // false: 写入操作完成后，数据可能仍留在操作系统缓冲区，如果进程崩溃，数据可能会丢失
    bool sync = false;  
};

}

#endif