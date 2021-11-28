#pragma once
#include "noncopyable.h"
#include <string.h>
#include <string>
namespace irono {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;
const int SIZE = 1000;

template <int SIZE>
class FixedBuffer : noncopyable {
public:
    FixedBuffer() : cur_(data_) {}
    ~FixedBuffer() {}

    void append(const char* buf, size_t len) {
        if (avail() > static_cast<int>(len)) {
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }
    int avail() const { return static_cast<int>(end() - cur_); }
    char* current() {return cur_;}
    void add(size_t len) { cur_ += len; }
    const char* data() const { return data_;}
    int length() const { return static_cast<int>(cur_ - data_); }
    void bzero() { memset(data_, 0, sizeof data_); }
    void reset() {cur_ = data_ ;};
private:
    const char* end() const { return data_ + sizeof data_;}
    char data_[SIZE];
    char* cur_;
};

class LogStream : noncopyable {
public:
    typedef FixedBuffer<kSmallBuffer> Buffer;
    template<typename T>
    void formatInteger(T);
    LogStream& operator<<(const char* str);
    LogStream& operator<<(const std::string& v);
    LogStream& operator<<(unsigned int);
    LogStream& operator<<(int v);
    LogStream& operator<<(long v);
    LogStream& operator<<(unsigned long v);
    LogStream& operator<<(char v);
    LogStream& operator<<(const void* v);
    const Buffer& buffer() const {return buffer_;}
private:
    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};







}
