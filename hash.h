// Computing file hashes.

#ifndef __HASH_H__
#define __HASH_H__

#include <string>

namespace asure {

  struct Hash {
    unsigned char data[16];
    operator std::string();

    // Set this hash to be the contents of the given file.
    void ofFile(std::string path);
  };

}

#endif
