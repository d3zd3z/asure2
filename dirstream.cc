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
  return EP(new FileEntry((*_priv)->name, _parent->getPath()));
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
  return EP(new DirEntry((*_priv)->name, _parent->getPath()));
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
