// Tree testing.

#ifndef __TEST__TEST_TREE_H__
#define __TEST__TEST_TREE_H__

#include <iostream>
#include <tr1/memory>
#include <vector>
#include "tree.hh"

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

class TestDirEntry;
typedef std::tr1::shared_ptr<TestDirEntry> TestDirEntryProxy;
class TestDirEntry : public DirEntry {
 public:
  TestDirEntry(std::string const& name, std::string const& path, Atts const& atts,
               std::vector<TestDirEntryProxy>& subdirs,
               std::vector<TestEntryProxy>& subfiles) :
      DirEntry(name, path),
      subdirs_(subdirs),
      subfiles_(subfiles)
  {
    atts_ = atts;
  }
  virtual ~TestDirEntry() { }

  virtual dir_iterator dirIter();
  virtual file_iterator fileIter();

 protected:
  std::vector<TestDirEntryProxy> const subdirs_;
  std::vector<TestEntryProxy> const subfiles_;

  virtual void computeAtts() { }
  virtual void computeExpensiveAtts() { }
};

}
}

#endif
