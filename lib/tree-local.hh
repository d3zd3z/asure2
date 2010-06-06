// Local directory trees.

#ifndef __TREE_LOCAL_H__
#define __TREE_LOCAL_H__

#include <list>
#include <string>
#include "tree.hh"

namespace asure {
namespace tree {

// Return a newly allocated NodeIterator that traverses a directory in the
// filesystem.  The iterator should be returned with delete when finished.
NodeIterator* walkTree(std::string const& path);

// A ListIterator the provides some assistance in building the list, namely a
// backdoor to access the list.
template <class E>
class ListIteratorBuilder : public ListIterator<E> {
 public:
  ListIteratorBuilder() : ListIterator<E>(std::list<E>()) { }
  ~ListIteratorBuilder() { }

  std::list<E>& getList() { return ListIterator<E>::list_; }
};

class LocalDirEntry;
typedef std::tr1::shared_ptr<LocalDirEntry> LocalDirEntryProxy;
class LocalDirEntry : public DirEntry {
 public:
  LocalDirEntry(std::string const& name, std::string const& path,
                struct stat& stat);
  virtual ~LocalDirEntry() { }

  virtual dir_iterator& dirIter() { scanDirectory(); return dirs_; }
  virtual file_iterator& fileIter() { scanDirectory(); return files_; }

  static DirEntryProxy readDir(std::string const& path);
 protected:
  void computeAtts() { }
  void computeExpensiveAtts() { }

 private:
  bool computed_;
  ListIteratorBuilder<EntryProxy> files_;
  ListIteratorBuilder<DirEntryProxy> dirs_;

  void scanDirectory();
};

}
}

#endif
