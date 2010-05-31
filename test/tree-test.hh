// Tree testing.

#ifndef __TEST__TEST_TREE_H__
#define __TEST__TEST_TREE_H__

#include <iostream>
#include <tr1/memory>
#include <list>
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
               std::list<DirEntryProxy>& subdirs,
               std::list<EntryProxy>& subfiles) :
      DirEntry(name, path),
      subdirs_(ListIterator<DirEntryProxy>(subdirs)),
      subfiles_(ListIterator<EntryProxy>(subfiles))
  {
    atts_ = atts;
  }
  virtual ~TestDirEntry() { }

  virtual dir_iterator& dirIter() { return subdirs_; }
  virtual file_iterator& fileIter() { return subfiles_; }

 protected:
  ListIterator<DirEntryProxy> subdirs_;
  ListIterator<EntryProxy> subfiles_;

  virtual void computeAtts() { }
  virtual void computeExpensiveAtts() { }
};

}
}

#endif
