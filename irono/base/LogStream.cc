#include "LogStream.h"
#include <string>
#include <string.h>
#include <algorithm>
using namespace irono;

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
template <typename T>
size_t convert(char buf[], T value) {
    T i = value;
    char* p = buf;

    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0) {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}
LogStream& LogStream::operator<<(const char* str) {
    if (str) {
        // buffer_.append(str, strlen(str));
        buffer_.append(str, strlen(str));
    }
    else {
        buffer_.append("(null)", 6);
    }
    return *this;
}
LogStream& LogStream::operator<<(const std::string& v) {
    buffer_.append(v.c_str(), v.size());
    return *this;
}
LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
}
LogStream& LogStream::operator<<(char v) {
    buffer_.append(&v, 1);
    return *this;
}
void LogStream::formatInteger(int v) {
    if (buffer_.avail() >= kMaxNumericSize) {
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}