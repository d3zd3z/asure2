// Computing file hashes.
//
extern "C" {
#include <sys/types.h>
#include <sys/fcntl.h>
#include <openssl/md5.h>
#include <errno.h>
}

#include <memory>
#include "hash.hh"
#include "exn.hh"

namespace asure {

class Buffer {
  public:
    static const int bufsize = 4096;
    unsigned char* get() { return data_; }
    Buffer() : data_(new unsigned char[bufsize]) { }
    ~Buffer() { delete[] data_; }
  private:
    unsigned char* data_;

    Buffer(const Buffer&);
    Buffer& operator=(const Buffer&);
};

class FileCloser {
 public:
  FileCloser(int fd) : fd_(fd) { }
  ~FileCloser() {
    close(fd_);
  }
 private:
  int fd_;
};

void
Hash::ofFile(std::string path)
{
  MD5_CTX ctx;
  MD5_Init(&ctx);

  Buffer buffer;

  errno = 0;
  int fd = open(path.c_str(), O_RDONLY | O_NOATIME);
  if (fd < 0 && errno == EPERM)
    fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    BOOST_THROW_EXCEPTION(io_error() << errno_code(errno) << path_code(path));
  }
  FileCloser cleanfd(fd);
  while (true) {
    ssize_t len = read(fd, buffer.get(), buffer.bufsize);
    if (len < 0)
      BOOST_THROW_EXCEPTION(io_error() << errno_code(errno) << path_code(path));
    if (len == 0)
      break;
    MD5_Update(&ctx, buffer.get(), len);
  }

  MD5_Final(data, &ctx);
}

namespace {
inline char itoc(unsigned char val)
{
  if (val < 10)
    return val + '0';
  else
    return val - 10 + 'a';
}
}

Hash::operator std::string()
{
  std::string buf(32, 'X');
  for (int i = 0; i < 16; i++) {
    buf[2*i] = itoc(data[i] >> 4);
    buf[2*i+1] = itoc(data[i] & 0x0f);
  }
  return buf;
}

}
