// Tree testing.

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "tree-test.h"
#include "tree-local.h"

namespace asure {

namespace tree {

static void printAtts(EntryProxy self, std::ostream& out)
{
  typedef Entry::Atts::const_iterator iter;
  const Entry::Atts& atts = self->getAtts();

  iter end = atts.end();
  for (iter i = atts.begin(); i != end; ++i) {
    out << " (:";
    out << i->first;
    out << ' ';
    out << i->second;
    out << ')';
  }
}

static void printFileSexp(EntryProxy self, std::ostream& out)
{
  out << "(file ";
  out << self->getName();
  printAtts(self, out);
  out << ')';
}

template <class iter>
static void showFiles(std::ostream& out, char const* name, iter begin, iter end)
{
  out << " (" << name;
  for (iter i = begin; i != end; ++i) {
    printFileSexp(*i, out);
  }
  out << ')';
}

template <class iter>
static void showSub(std::ostream& out, char const* name, iter begin, iter end);

template <class D>
static void printSexp(D self, std::ostream& out)
{
  out << "(dir ";
  out << self->getName();
  printAtts(self, out);

  showSub(out, "subdirs ", self->dirBegin(), self->dirEnd());
  showFiles(out, "files ", self->fileBegin(), self->fileEnd());

  out << ')';
}

template<class iter>
static void showSub(std::ostream& out, char const* name, iter begin, iter end)
{
  out << " (" << name;
  for (iter i = begin; i != end; ++i) {
    printSexp(*i, out);
  }
  out << ')';
}

}

namespace test {

using namespace asure::tree;

BOOST_AUTO_TEST_SUITE(tree);
BOOST_AUTO_TEST_CASE(simple)
{
  typedef Entry::Atts Atts;
  typedef Entry::Atts::value_type value;

  Atts m1;
  m1["kind"] = "reg";

  Atts m2;
  m2["kind"] = "dir";

  std::vector<TestDirEntryProxy> subdirs;
  std::vector<TestEntryProxy> subfiles;

  subfiles.push_back(TestEntryProxy(new TestEntry("file1", "dir1/file1", m1)));

  TestDirEntryProxy dir1(new TestDirEntry("dir1", "dir1", m2, subdirs, subfiles));

  subfiles.clear();
  subdirs.clear();
  subdirs.push_back(dir1);
  TestDirEntryProxy root(new TestDirEntry("__root__", "", m2, subdirs, subfiles));

  std::stringstream ss;
  printSexp(root, ss);
  BOOST_CHECK_EQUAL(ss.str(),
                    "(dir __root__ (:kind dir) (subdirs (dir dir1 (:kind dir) (subdirs ) "
                    "(files (file file1 (:kind reg))))) "
                    "(files ))");
}

BOOST_AUTO_TEST_CASE(local)
{
  // This test depends on the contents of /tmp, so don't normally run it.
  return;
  // LocalDirEntry root("__root__", "/tmp", rootStat);
  LocalDirEntryProxy root = LocalDirEntry::readDir("/tmp");

  std::stringstream ss;
  printSexp(root, ss);
  std::cout << ss.str() << '\n';
}

BOOST_AUTO_TEST_SUITE_END();

}
}
