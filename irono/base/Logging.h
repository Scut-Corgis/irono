#pragma once

#include "Timestamp.h"
#include "LogStream.h"
#include <string>
namespace irono {


class Logger {
public:
    enum LogLevel {
        TRACE,
        DEBUG,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(const char* filename, int line, LogLevel level);
    ~Logger();

    LogStream& stream() {return impl_.stream_;}
    static LogLevel logLevel();
    static std::string getLogFileName() { return logFileName_; }
    typedef void (*OutputFunc)(const char* msg, int len);

private:
    class Impl {
    public:
        Impl(const char* filename, int line, LogLevel level);
        void formatTime();

        LogStream stream_;
        LogLevel level_;
        int line_;
        std::string filename_;
    };

    Impl impl_;
    static std::string logFileName_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}



#define LOG_TRACE if (irono::Logger::LogLevel() <= irono::Logger::TRACE) \
    irono::Logger(__FILE__, __LINE__, irono::Logger::TRACE).stream()
#define LOG_DEBUG if (irono::Logger::LogLevel() <= irono::Logger::DEBUG) \
    irono::Logger(__FILE__, __LINE__, irono::Logger::DEBUG).stream()
#define LOG_FATAL if (irono::Logger::LogLevel() <= irono::Logger::FATAL) \
    irono::Logger(__FILE__, __LINE__, irono::Logger::FATAL).stream()


#define CHECK_NOTNULL(val) \
  irono::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

// A small helper for CHECK_NOTNULL().
template <typename T>
T* CheckNotNull(const char *file, int line, const char *names, T* ptr) {
  if (ptr == NULL) {
   Logger(file, line, Logger::FATAL).stream() << names;
  }
  return ptr;
}

const char* strerror_tl(int savedErrno);

}