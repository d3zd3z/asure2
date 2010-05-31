// Directory streams.

#include <errno.h>

#include <sstream>
#include <iostream>

#include "dir.hh"
#include "dirstream.hh"
#include "hash.hh"

using std::string;
using std::tr1::shared_ptr;

namespace asure {
namespace stream {

shared_ptr<FsDirSource>
DirEntry::getDirSource()
{
  if (_name.length() > 0)
    return FsDirSource::walkFsDir(_path + '/' + _name);
  else
    return FsDirSource::walkFsDir(_path);
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
  const struct stat& stat = node_->stat;
  if (S_ISREG(stat.st_mode)) {
    _atts["kind"] = "file";
    _atts["uid"] = lltoa(stat.st_uid);
    _atts["gid"] = lltoa(stat.st_gid);
    _atts["mtime"] = lltoa(stat.st_mtime);
    _atts["ctime"] = lltoa(stat.st_ctime);
    _atts["ino"] = lltoa(stat.st_ino);
    _atts["perm"] = lltoa(stat.st_mode & ~S_IFMT);
  } else if (S_ISLNK(stat.st_mode)) {
    _atts["kind"] = "lnk";
  } else {
    std::cerr << "Unimplemented file type: " << (stat.st_mode & S_IFMT) << '\n';
    assert(false);
  }
}

static string
getLink(string path, int length)
{
  char buf[length];
  ssize_t len = readlink(path.c_str(), buf, length);
  if (len < 0)
    throw errno;
  else if (len < length)
    return string(buf, len);
  else
    return getLink(path, 2*length);
}

void
LocalFileEntry::computeExpensiveAtts()
{
  const struct stat& stat = node_->stat;
  if (S_ISREG(stat.st_mode)) {
    Hash h;
    h.ofFile(_path + "/" + _name);
    _atts["md5"] = h;
  } else if (S_ISLNK(stat.st_mode)) {
    string target = getLink(_path + "/" + _name, 128);
    _atts["targ"] = target;
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
  const struct stat& stat = node_->stat;
  _atts["kind"] = "dir";
  _atts["uid"] = lltoa(stat.st_uid);
  _atts["gid"] = lltoa(stat.st_gid);
  _atts["perm"] = lltoa(stat.st_mode & ~S_IFMT);
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
  // Note the special marker of the empty string for the root.
  return DirEntryProxy(new LocalDirEntry("", path, DirNode::getDir(path)));
}

}
}
