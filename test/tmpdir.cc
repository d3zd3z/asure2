// Temporary directory testing.

#include <boost/test/unit_test.hpp>

extern "C" {
#include <sys/stat.h>
#include <sys/types.h>
}

#include <cassert>
#include <cstdlib>
#include <sstream>
#include "uuid.h"
#include "tmpdir.hh"

namespace asure {
namespace test {

namespace {
std::string makePath(int count = 5)
{
  std::stringstream buf;

  buf << "/tmp/test-";
  uuid_t id;
  uuid_generate_random(id);
  char text[37];
  uuid_unparse_lower(id, text);
  buf << text;

  const std::string path = buf.str();
  int result = mkdir(path.c_str(), 0700);
  if (result != 0) {
    assert(count > 0);
    return makePath(count - 1);
  }
  return path;
}
}

TmpDir::TmpDir() : path_(makePath())
{
}

TmpDir::~TmpDir()
{
  int result = system(("rm -rf " + path_).c_str());
  assert(result == 0);
}

BOOST_AUTO_TEST_SUITE(tmpdir);
BOOST_AUTO_TEST_CASE(cleanup)
{
  std::string tmpName;
  {
    TmpDir tmp;
    tmpName = tmp.getPath();
    BOOST_CHECK_EQUAL(mkdir((tmp.getPath() + "/subdir").c_str(), 0700), 0);
    BOOST_CHECK_EQUAL(system(("touch " + tmpName + "/subdir/myfile").c_str()), 0);
  }
  struct stat sbuf;
  BOOST_CHECK_EQUAL(lstat(tmpName.c_str(), &sbuf), -1);
}
BOOST_AUTO_TEST_SUITE_END();

}}
