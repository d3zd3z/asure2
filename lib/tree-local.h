// Local directory trees.

#ifndef __TREE_LOCAL_H__
#define __TREE_LOCAL_H__

#include <vector>
#include <string>
#include "tree.h"

namespace asure {
namespace tree {

struct LocalInfo;
typedef DirEntry<LocalInfo> LocalDirEntry;
typedef std::tr1::shared_ptr<LocalDirEntry> LocalDirEntryProxy;

struct LocalInfo {
  typedef std::vector<LocalDirEntryProxy>::iterator dir_iterator;
  typedef std::vector<EntryProxy>::iterator file_iterator;
};

extern void x(char const* msg);
template <>
class DirEntry<LocalInfo> : public Entry {
 public:
  typedef LocalInfo::dir_iterator dir_iterator;
  dir_iterator dirBegin() { scanDirectory(); return dirs_.begin(); }
  dir_iterator dirEnd() { scanDirectory(); return dirs_.end(); }

  typedef LocalInfo::file_iterator file_iterator;
  file_iterator fileBegin() { scanDirectory(); return files_.begin(); }
  file_iterator fileEnd() { scanDirectory(); return files_.end(); }

  DirEntry(std::string const& name, std::string const& path,
           struct stat& stat);
  virtual ~DirEntry() { }

  static LocalDirEntryProxy readDir(std::string const& path);
 protected:
  void computeAtts() { }
  void computeExpensiveAtts() { }

 private:
  bool computed_;
  std::vector<EntryProxy> files_;
  std::vector<LocalDirEntryProxy> dirs_;

  void scanDirectory();
};

}
}

#endif
