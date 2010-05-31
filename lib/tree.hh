// Directory trees.

#ifndef __TREE_H__
#define __TREE_H__

#include <map>
#include <string>
#include <tr1/memory>

namespace asure {
namespace tree {

// Everything in the filesystem is represented by an Entry.  An entry
// has a general set of attributes (key/value pairs entirely of
// strings).  These attributes are further broken down into those that
// are cheap to compute, and those that are more expensive (such as
// the hash of a file).  These expensive attributes can be optionally
// copied from another entry, provided that sufficient information
// exists.
//
// Each Entry carries with a simple name, which is the name of that
// entry in a directory tree, as well as a full path that leads to
// that Entry.
class Entry {
 public:
  typedef std::map<std::string, std::string> Atts;

  virtual ~Entry() { }

  Entry(const std::string& name, const std::string& path) :
      name_(name), path_(path), atts_(), attsComputed_(false)
  { }

  const std::string& getName() const { return name_; }
  const std::string& getPath() const { return name_; }

  const Atts& getAtts() const {
    if (!attsComputed_) {
      // The attribute computations can be considered a cache
      // update, which maintains the illusion of const.
      Entry* mthis = const_cast<Entry*>(this);
      mthis->computeAtts();
      mthis->computeExpensiveAtts();
      mthis->attsComputed_ = true;
    }
    return atts_;
  }

 protected:
  virtual void computeAtts() = 0;
  virtual void computeExpensiveAtts() = 0;

  const std::string name_;
  const std::string path_;

  Atts atts_;
  bool attsComputed_;
};
typedef std::tr1::shared_ptr<Entry> EntryProxy;

// This trait marks anything that is a DirEntry, since the real DirEntries do
// not form any kind of heirarchy.
// class DirEntryTrait { };

// Some entries also contain other entries (directories).  These can
// be iterated over in a limited manner.  The limitations are:
// 1.  The iterators may be used once.
// 2.  The dir iterators must be used before the file iterators.
template <class S>
class OldDirEntry : public Entry {
 public:
  // Iterate over the subdirectories of this Entry.
  typedef typename S::dir_iterator dir_iterator;
  dir_iterator dirBegin();
  dir_iterator dirEnd();

  typedef typename S::file_iterator file_iterator;
  file_iterator fileBegin();
  file_iterator fileEnd();

 private:
  OldDirEntry() { }
};

// Implementation notes:
// The iterators for dir_iterator and file_iterator are only defined
// for: operator!=, operator==, operator++(), and operator*().  If the
// implementations are constrained on traversal rules, it should
// enforce that constraint.

}
}

#endif
