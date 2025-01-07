#include "env.h"

#include <cstdarg>

namespace kvstorage {

Status Env::deleteDir(const std::string& fname) { return removeDir(fname); }
Status Env::deleteFile(const std::string& fname) { return removeFile(fname); }

void Log(Logger* info_log, const char* format, ...) {
    if (info_log != nullptr) {
        std::va_list ap;
        va_start(ap, format);
        info_log->logv(format, ap);
        va_end(ap);
    }
}

static Status DoWriteStringToFile(Env* env, const Slice& data, const std::string& frame, bool may_sync) {
    WritableFile* file;
    Status s = env->newWritableFile(frame, &file);
    if (!s.ok()) {
        return s;  // 创建文件失败
    }
    s = file->append(data);
    if (s.ok() && may_sync) {
        s = file->sync();
    }
    if (s.ok()) {
        s = file->close();
    }
    delete file;  // 关闭文件
    if (!s.ok()) {
        env->deleteFile(frame);  // 如果前面的文件操作失败，则删除文件
    }
    return s;
}

Status WriteStringToFile(Env* env, const Slice& data, const std::string& fname) {
    return DoWriteStringToFile(env, data, fname, false);
}

// 从指定文件中读取数据
Status ReadFileToString(Env* env, const std::string& fname, std::string* data) {
    data->clear();
    SequentialFile* file;
    Status s = env->newSequentialFile(fname, &file);
    if (!s.ok()) {
        return s;
    }
    static const int s_buffer_size = 8192;
    char* space = new char[s_buffer_size];  // 临时存储空间
    while (true) {
        Slice fragment;
        s = file->read(s_buffer_size, &fragment, space);
        if (!s.ok()) {
            break;
        }
        data->append(fragment.data(), fragment.size());
        if (fragment.empty()) {
            break;
        }
    }
    delete[] space;
    delete file;
    return s;
}

}  // namespace kvstorage