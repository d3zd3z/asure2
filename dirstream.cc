// Directory streams.

#include "dir.h"
#include "dirstream.h"

using std::string;
using std::tr1::shared_ptr;

namespace asure {
namespace stream {

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
  return EP(new DirEntry((*_priv)->name, _parent->getPath()));
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

}
}
