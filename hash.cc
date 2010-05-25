// Computing file hashes.

#include <memory>
#include "hash.h"

extern "C" {
#include <sys/types.h>
#include <sys/fcntl.h>
#include <openssl/md5.h>
}

class Buffer {
  public:
    static const int bufsize = 4096;
    unsigned char *data;
    Buffer() { data = new unsigned char[bufsize]; }
    ~Buffer() { delete[] data; }
};

void hash_file(std::string path, file_hash& hash)
{
  MD5_CTX ctx;
  MD5_Init(&ctx);

  Buffer buffer;

  // TODO: Try with O_NOATIME, and then retry without it, if it fails.
  int fd = open(path.c_str(), O_RDONLY | O_NOATIME);
  if (fd < 0)
    throw fd;
  while (true) {
    ssize_t len = read(fd, buffer.data, buffer.bufsize);
    if (len < 0)
      throw len;
    if (len == 0)
      break;
    MD5_Update(&ctx, buffer.data, len);
  }
  close(fd);

  MD5_Final(hash.data, &ctx);
}

static inline char itoc(unsigned char val)
{
  if (val < 10)
    return val + '0';
  else
    return val - 10 + 'a';
}

file_hash::operator std::string()
{
  std::string buf(32, 'X');
  for (int i = 0; i < 16; i++) {
    buf[2*i] = itoc(data[i] >> 4);
    buf[2*i+1] = itoc(data[i] & 0x0f);
  }
  return buf;
}
