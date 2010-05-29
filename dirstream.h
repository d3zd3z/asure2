// Streamed information about directory contents.
//
// Represents a linearization of a directory traversal.

#ifndef __DIRSTREAM_H__
#define __DIRSTREAM_H__

#include <cassert>
#include <iterator>
#include <string>
#include <map>
#include <tr1/memory>

#include "dir.h"

namespace asure {
namespace stream {

class Entry {
  public:
    virtual ~Entry() { }

    Entry(const std::string& name, const std::string& path)
      : _name(name), _path(path), _attsComputed(false) { }
    const std::string& getName() const { return _name; }

    typedef std::map<std::string, std::string> Atts;

    const Atts& getAtts() {
      if (!_attsComputed) {
	this->computeAtts();
	this->computeExpensiveAtts();
	_attsComputed = true;
      }
      return _atts;
    }

    // Can be used to possibly pre-compute atts, if some information
    // is correct (e.g. it is determined to be a completely unchanged
    // file).
    virtual void cloneAtts(Entry& other) {
      // The base version just makes sure that the attributes haven't
      // been fetched.
      assert(!_attsComputed);
    }

    // This is just temporary.
    const std::string& getPath() const { return _path; }
  protected:
    const std::string _name;
    const std::string _path;

    // These are the file attributes.
    Atts _atts;
    bool _attsComputed;

    virtual void computeAtts() = 0;
    virtual void computeExpensiveAtts() = 0;
};
typedef std::tr1::shared_ptr<Entry> EntryProxy;

class FsDirSource;
typedef std::tr1::shared_ptr<FsDirSource> FsDirSourceProxy;
class DirEntry : public Entry {
  public:
    DirEntry(const std::string& name, const std::string& path)
      : Entry(name, path) { }

    // Get the dir source for reading in more of these.
    FsDirSourceProxy getDirSource();
};
typedef std::tr1::shared_ptr<DirEntry> DirEntryProxy;

// An iterator over source type 'S', returning entries of type 'E'.
// Note that the iterators don't hold the reference to the parent, so
// it's important for that to not go out of scope (gc is nicer, isn't
// it).
template <typename S, typename E, typename P>
class EntryIterator /*iterator*/ {
  public:
    typedef typename std::tr1::shared_ptr<E> EP;
    EntryIterator(S* const parent, const P& priv) : _parent(parent), _priv(priv) { }

    // Iterator operations.
    bool operator!=(const EntryIterator& other) const;
    bool operator==(const EntryIterator& other) const;
    EntryIterator& operator++();
    EP operator*() const;
  protected:
    const S* _parent;
    P _priv;
};

// A DirSource iterates through some kind of directory tree in a
// predefined order.  The traversal is depth first, so directories are
// visited before other types of nodes.  Within a directory, the
// regular files follow the directories, and these are followed by the
// other types of nodes.
//
// Note that the iterators are valid only as long as this object is
// still allocated.
class FsDirSource {
  public:
    FsDirSource(const std::string& path)
      : _path(path), real(path), cachedFileEnd(real.getNonDirs().end()) { }
    // ~FsDirSource();

    typedef EntryIterator<FsDirSource, Entry, std::vector<DirNodeProxy>::const_iterator > FileIterator;
    FileIterator fileBegin();
    FileIterator fileEnd();

    typedef EntryIterator<FsDirSource, DirEntry, std::vector<DirNodeProxy>::const_iterator > DirIterator;
    DirIterator dirBegin();
    DirIterator dirEnd();

    static FsDirSourceProxy walkFsDir(std::string path);

    const std::string& getPath() const { return _path; }

  private:
    const std::string _path;
    const Directory real;

    // Cached endings.
    const std::vector<DirNodeProxy>::const_iterator cachedFileEnd;
};

// This starts by walking from the top of some directory.
DirEntryProxy walkPath(std::string& path);

}
}

#endif
