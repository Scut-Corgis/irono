/*
 * @Author: your name
 * @Date: 2021-11-27 15:11:25
 * @LastEditTime: 2021-12-01 15:31:15
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /irono/irono/base/LogFile.h
 */
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