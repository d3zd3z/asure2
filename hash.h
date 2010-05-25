// Computing file hashes.

#ifndef __HASH_H__
#define __HASH_H__

#include <string>

struct file_hash {
  unsigned char data[16];
  operator std::string();
};

void hash_file(std::string path, file_hash& hash);

#endif
