// Simple gzip IO streams.
// Very simple gzip IO streams.

#ifndef __LIB_GZSTREAM_HH__
#define __LIB_GZSTREAM_HH__

#include "zlib.h"
#include <ios>

#include "exn.hh"

namespace asure {

class gzstream {
 public:
  gzstream() : file(0) { }
  ~gzstream() {
    if (file != 0)
      gzclose(file);
    file = 0;
  }

  void open(char const* path, char const* flags) {
    file = gzopen(path, flags);
    if (file == 0)
      throw IO_error("gzstream::open", path);
  }

  void put(char ch) {
    gzputc(file, ch);
    // TODO: Detect errors.
  }

  void write(char const* chars, int len) {
    int offset = 0;
    while (len > 0) {
      int count = gzwrite(file, chars + offset, len);
      if (count <= 0)
        throw IO_error("gzstream::write", "unknown");
      offset += count;
      len -= count;
    }
  }

  void get(char& ch) {
    int tmp = gzgetc(file);
    if (tmp == -1)
      throw IO_error("gzstream::get", "unknown");
    ch = tmp;
  }

  void read(char* chars, int len) {
    int offset = 0;
    while (len > 0) {
      int count = gzread(file, chars + offset, len);
      if (count <= 0)
        throw IO_error("gzstream::read", "unknown");
      offset += count;
      len -= count;
    }
  }

  void close() {
    if (file != 0) {
      gzclose(file);
      file = 0;
    }
  }

  bool isOpen() const { return file != 0; }

 private:
  gzFile file;
};

}

#endif
