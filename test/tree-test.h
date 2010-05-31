// Tree testing.

#ifndef __TEST__TEST_TREE_H__
#define __TEST__TEST_TREE_H__

#include <iostream>
#include <tr1/memory>
#include <vector>
#include "tree.h"

namespace asure {
namespace tree {

class TestEntry : public Entry {
 public:
  TestEntry(const std::string& name, const std::string& path, const Atts& atts) :
      Entry(name, path)
  {
    atts_ = atts;
  }
 protected:
  void computeAtts() { }
  void computeExpensiveAtts() { }
};
typedef std::tr1::shared_ptr<TestEntry> TestEntryProxy;

struct TestInfo;
typedef DirEntry<TestInfo> TestDirEntry;
typedef std::tr1::shared_ptr<TestDirEntry> TestDirEntryProxy;

struct TestInfo {
  typedef std::vector<TestDirEntryProxy>::const_iterator dir_iterator;
  typedef std::vector<TestEntryProxy>::const_iterator file_iterator;
};

template <>
class DirEntry<TestInfo> : public TestEntry {
 public:
  typedef TestInfo::dir_iterator dir_iterator;
  dir_iterator dirBegin() { return subdirs_.begin(); }
  dir_iterator dirEnd() { return subdirs_.end(); }

  typedef TestInfo::file_iterator file_iterator;
  file_iterator fileBegin() { return subfiles_.begin(); }
  file_iterator fileEnd() { return subfiles_.end(); }

  DirEntry(const std::string& name, const std::string& path, const Atts& atts,
           const std::vector<TestDirEntryProxy>& subdirs,
           const std::vector<TestEntryProxy>& subfiles) :
      TestEntry(name, path, atts),
      subdirs_(subdirs),
      subfiles_(subfiles)
  {
    atts_ = atts;
  }
 protected:
  const std::vector<TestDirEntryProxy> subdirs_;
  const std::vector<TestEntryProxy> subfiles_;
  void computeAtts() { }
  void computeExpensiveAtts() { }
};

}
}

#endif
