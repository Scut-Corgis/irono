#pragma once
#include <string>
#include  "noncopyable.h"
namespace irono {

class AppendFile : noncopyable {
public:
    explicit AppendFile(std::string filename);
    ~AppendFile();
    void append(const char* logline, const size_t len);
    void flush();

private:
    size_t write(const char* logline, size_t len);
    FILE* fp_;
    char buffer_[64 * 1024];
};





} //namespace irono
