#include "LogStream.h"
#include <string>
#include <string.h>
#include <algorithm>
using namespace irono;

const char digits[] = "9876543210123456789";
const char digitsHex[] = "0123456789ABCDEF";
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
size_t convertHex(char buf[], uintptr_t value)
{
  uintptr_t i = value;
  char* p = buf;

  do
  {
    int lsd = static_cast<int>(i % 16);
    i /= 16;
    *p++ = digitsHex[lsd];
  } while (i != 0);

  *p = '\0';
  std::reverse(buf, p);

  return p - buf;
}

LogStream& LogStream::operator<<(const void* p)
{
  uintptr_t v = reinterpret_cast<uintptr_t>(p);
  if (buffer_.avail() >= kMaxNumericSize)
  {
    char* buf = buffer_.current();
    buf[0] = '0';
    buf[1] = 'x';
    size_t len = convertHex(buf+2, v);
    buffer_.add(len+2);
  }
  return *this;
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