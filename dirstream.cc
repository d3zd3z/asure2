// Directory streams.

#include <sstream>
#include "dir.h"
#include "dirstream.h"
#include "hash.h"

using std::string;
using std::tr1::shared_ptr;

namespace asure {
namespace stream {

shared_ptr<FsDirSource>
DirEntry::getDirSource()
{
  return FsDirSource::walkFsDir(_path + '/' + _name);
}


shared_ptr<FsDirSource>
FsDirSource::walkFsDir(const string path)
{
  return shared_ptr<FsDirSource>(new FsDirSource(path));
}


//////////////////////////////////////////////////////////////////////
// File sources use this class to compute the attributes.
class LocalFileEntry : public Entry {
  public:
    LocalFileEntry(const std::string& name, const std::string& path, const DirNodeProxy& node)
      : Entry(name, path), node_(node) { }
  protected:
    virtual void computeAtts();
    virtual void computeExpensiveAtts();
    const DirNodeProxy node_;
};

static string
lltoa(long long value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

void
LocalFileEntry::computeAtts()
{
  typedef Atts::value_type value;
  const struct stat& stat = node_->stat;
  if (S_ISREG(stat.st_mode)) {
    _atts.insert(value("kind", "file"));
    _atts.insert(value("uid", lltoa(stat.st_uid)));
    _atts.insert(value("gid", lltoa(stat.st_gid)));
    _atts.insert(value("mtime", lltoa(stat.st_mtime)));
    _atts.insert(value("ctime", lltoa(stat.st_ctime)));
    _atts.insert(value("ino", lltoa(stat.st_ino)));
    _atts.insert(value("perm", lltoa(stat.st_mode & ~S_IFMT)));
  } else {
    assert(false);
  }
}

void
LocalFileEntry::computeExpensiveAtts()
{
  typedef Atts::value_type value;
  const struct stat& stat = node_->stat;
  if (S_ISREG(stat.st_mode)) {
    Hash h;
    h.ofFile(_path + "/" + _name);
    _atts.insert(value("md5", h));
  }
}

FsDirSource::FileIterator
FsDirSource::fileBegin()
{
  return FileIterator(this, real.getNonDirs().begin());
}

FsDirSource::FileIterator
FsDirSource::fileEnd()
{
  return FileIterator(this, cachedFileEnd);
}

template <>
FsDirSource::FileIterator::EP
FsDirSource::FileIterator::operator*() const
{
  return EP(new LocalFileEntry((*_priv)->name, _parent->getPath(), *_priv));
}

template <>
bool
FsDirSource::FileIterator::operator!=(const FsDirSource::FileIterator& other) const
{
  return _priv != other._priv;
}

template <>
FsDirSource::FileIterator&
FsDirSource::FileIterator::operator++()
{
  _priv++;
  return *this;
}

//////////////////////////////////////////////////////////////////////
// Local directory sources.
class LocalDirEntry : public DirEntry {
  public:
    LocalDirEntry(const std::string& name, const std::string& path, const DirNodeProxy& node)
      : DirEntry(name, path), node_(node) { }
  protected:
    virtual void computeAtts();
    virtual void computeExpensiveAtts() { }
    const DirNodeProxy node_;
};

void
LocalDirEntry::computeAtts()
{
  typedef Atts::value_type value;
  const struct stat& stat = node_->stat;
  _atts.insert(value("kind", "dir"));
  _atts.insert(value("uid", lltoa(stat.st_uid)));
  _atts.insert(value("gid", lltoa(stat.st_gid)));
  _atts.insert(value("perm", lltoa(stat.st_mode & ~S_IFMT)));
}

FsDirSource::DirIterator
FsDirSource::dirBegin()
{
  return DirIterator(this, real.getDirs().begin());
}

FsDirSource::DirIterator
FsDirSource::dirEnd()
{
  return DirIterator(this, real.getDirs().end());
}

// TODO: Figure out how to now copy this block of code.  Copying is
// bad.
template <>
FsDirSource::DirIterator::EP
FsDirSource::DirIterator::operator*() const
{
  return EP(new LocalDirEntry((*_priv)->name, _parent->getPath(), *_priv));
}

template <>
bool
FsDirSource::DirIterator::operator!=(const FsDirSource::DirIterator& other) const
{
  return _priv != other._priv;
}

template <>
FsDirSource::DirIterator&
FsDirSource::DirIterator::operator++()
{
  _priv++;
  return *this;
}

DirEntryProxy
walkPath(string& path)
{
  return DirEntryProxy(new LocalDirEntry(path, path, DirNode::getDir(path)));
}

}
}
