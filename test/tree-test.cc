// Tree testing.

#include <iostream>
#include <boost/test/unit_test.hpp>
#include "tree-test.h"

namespace asure {

namespace tree {

void TestEntry::printAtts(std::ostream& out)
{
  typedef Atts::const_iterator iter;
  iter end = atts_.end();
  for (iter i = atts_.begin(); i != end; ++i) {
    out << " (:";
    out << i->first;
    out << ' ';
    out << i->second;
    out << ')';
  }
}

void TestEntry::printSexp(std::ostream& out)
{
  out << "(file ";
  out << getName();
  printAtts(out);
  out << ')';
}

template<class iter>
static void showSub(std::ostream& out, char const* name, iter begin, iter end)
{
  out << " (" << name;
  for (iter i = begin; i != end; ++i) {
    (*i)->printSexp(out);
  }
  out << ')';

}

void TestDirEntry::printSexp(std::ostream& out)
{
  out << "(dir ";
  out << getName();
  printAtts(out);

  showSub(out, "subdirs ", dirBegin(), dirEnd());
  showSub(out, "files ", fileBegin(), fileEnd());

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
  root->printSexp(ss);
  BOOST_CHECK_EQUAL(ss.str(),
                    "(dir __root__ (:kind dir) (subdirs (dir dir1 (:kind dir) (subdirs ) "
                    "(files (file file1 (:kind reg))))) "
                    "(files ))");
}
BOOST_AUTO_TEST_SUITE_END();

}
}
