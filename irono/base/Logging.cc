#include "Logging.h"
#include <string>
#include <sys/time.h>
#include "CurrentThread.h"
#include "AsyncLogging.h"
namespace irono {

__thread char t_errnobuf[512];
// __thread char t_time[32];
// __thread time_t t_lastSecond;


//将errno用string打印出来具体错误是什么
const char* strerror_tl(int savedErrno)
{
  return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}





std::string Logger::logFileName_ = "./irono.log";
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging* AsyncLogger_;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
  "TRACE ",
  "DEBUG ",
  "INFO  ",
  "ERROR ",
  "FATAL ",
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
    if (impl_.level_ == FATAL) {
    //等待几秒是为了等前端缓冲区刷到后端去后，再flush()写到磁盘，不等待就会失败，目前还没想到怎么改
    sleep(5);
    AsyncLogger_->output().flush();
    abort();
  }
}

Logger::Impl::Impl(const char* filename, int line, LogLevel level) 
    : stream_(),
      level_(level),
      line_(line),
      filename_(filename)
{
    formatTime();
    stream_<<LogLevelName[level];
    CurrentThread::tid();
    stream_<<CurrentThread::t_tidString<<" ";
}

//localtime最多精确到秒,如果想要微秒也不难，tv.tv_usec就是微妙，可以参考Timestamp::toFormattedString()的设计
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

void Logger::setOutput(OutputFunc out) {
    g_output = out;
}

}//namespace irono
