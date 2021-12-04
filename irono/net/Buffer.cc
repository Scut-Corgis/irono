
#include "Buffer.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

#include <errno.h>
#include <memory.h>
#include <sys/uio.h>

using namespace irono;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    //64MB的栈临时缓冲区
  char extrabuf[65536];
  struct iovec vec[2];
  const size_t writable = writableBytes();
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  const ssize_t n = readv(fd, vec, 2);
  if (n < 0) {
    *savedErrno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    writerIndex_ += n;
  } else {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }
  return n;
}

