// Streamed information about directory contents.
//
// Represents a linearization of a directory traversal.

#ifndef __DIRSTREAM_H__
#define __DIRSTREAM_H__

#include <iterator>
#include <string>
#include <tr1/memory>

#include "dir.h"

namespace asure {
namespace stream {

class Entry {
  public:
    Entry(const std::string& name, const std::string& path)
      : _name(name), _path(path) { }
    const std::string& getName() const { return _name; }

    // This is just temporary.
    const std::string& getPath() const { return _path; }
  protected:
    const std::string _name;
    const std::string _path;
};

class FsDirSource;
class DirEntry : public Entry {
  public:
    DirEntry(const std::string& name, const std::string& path)
      : Entry(name, path) { }
    std::tr1::shared_ptr<FsDirSource> getDirSource();
};

class FsDirSource;
class FileEntry : public Entry {
  public:
    FileEntry(const std::string& name, const std::string& path)
      : Entry(name, path) { }
};

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

    typedef EntryIterator<FsDirSource, FileEntry, std::vector<DirNode*>::const_iterator > FileIterator;
    FileIterator fileBegin();
    FileIterator fileEnd();

    typedef EntryIterator<FsDirSource, DirEntry, std::vector<DirNode*>::const_iterator > DirIterator;
    DirIterator dirBegin();
    DirIterator dirEnd();

    static std::tr1::shared_ptr<FsDirSource> walkFsDir(std::string path);

    const std::string& getPath() const { return _path; }

  private:
    const std::string _path;
    const Directory real;

    // Cached endings.
    const std::vector<DirNode*>::const_iterator cachedFileEnd;
};

}
}

#endif
