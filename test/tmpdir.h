// Framework support for temporary directories.

#ifndef __TEST_TMPDIR_H__
#define __TEST_TMPDIR_H__

#include <string>

namespace asure { namespace test {

class TmpDir {
  public:
    TmpDir();
    ~TmpDir();

    const std::string& getPath() { return path_; }
  private:
    const std::string path_;

    TmpDir(const TmpDir&);
    void operator=(const TmpDir&);
};

}}

#endif
