// Directory streams.

#include "dir.h"
#include "dirstream.h"

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
    LocalFileEntry(const std::string& name, const std::string& path)
      : Entry(name, path) { }
  protected:
    virtual void computeAtts();
    virtual void computeExpensiveAtts();
};

void
LocalFileEntry::computeAtts()
{
  typedef Atts::value_type value;
  assert(false);
}

void
LocalFileEntry::computeExpensiveAtts()
{
  assert(false);
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
  return EP(new LocalFileEntry((*_priv)->name, _parent->getPath()));
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
    LocalDirEntry(const std::string& name, const std::string& path)
      : DirEntry(name, path) { }
  protected:
    virtual void computeAtts();
    virtual void computeExpensiveAtts() { }
};

void
LocalDirEntry::computeAtts()
{
  assert(false);
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
  return EP(new LocalDirEntry((*_priv)->name, _parent->getPath()));
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

}
}
