#pragma once
#include "noncopyable.h"
#include <memory>
#include "Mutex.h"
#include "FileUtil.h"

namespace irono {

class LogFile : noncopyable {
public:
    LogFile(const std::string& basename, int flushEveryN = 1024);
    ~LogFile();

    void append(const char* logline, int len);
    void flush();

private:
    //底层使用无锁fwrite
    void append_unlocked(const char* logline, int len);

    const std::string basename_;
    const int flushEveryN_;

    int count_;
    std::unique_ptr<MutexLock> mutex_;
    std::unique_ptr<AppendFile> file_;
};

}