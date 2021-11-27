#include "Logging.h"
#include <string>
#include <sys/time.h>
#include "AsyncLogging.h"
namespace irono {

std::string Logger::logFileName_ = "./irono.log";
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging* AsyncLogger_;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
};

void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len , stdout);
}

Logger::Logger(const char* filename, int line, LogLevel level) 
    : impl_(filename, line, level)
{}


void once_init() {
    AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
    AsyncLogger_->start();
}

void output(const char* msg, int len) {
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}
Logger::OutputFunc g_output = output;

Logger::~Logger() {
    impl_.stream_ << " -- " << impl_.filename_ << ":" << impl_.line_ << '\n';
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
}

Logger::Impl::Impl(const char* filename, int line, LogLevel level) 
    : stream_(),
      level_(level),
      line_(line),
      filename_(filename)
{
    formatTime();
}

void Logger::Impl::formatTime() {
    struct timeval tv;
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, NULL);
    time = tv.tv_sec;
    struct tm* p_time = localtime(&time);
    strftime(str_t, 26, "%Y-%m-%d %H:%M:%S ", p_time);
    stream_ <<str_t;
}

}//namespace irono
