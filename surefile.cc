// Writing surefiles

#include <cassert>
#include <iostream>
#include "surefile.h"

namespace asure {

using std::string;

class Emitter {
  public:
    Emitter(const string& base) : base_(base), out_(open()) { }

    void walk(stream::DirEntryProxy root);

    void putChar(char ch) { out_->put(ch); }
    void putInt(int val);
    void putString(const string& str) {
      putInt(str.length());
      out_->write(str.data(), str.length());
    }
  private:
    const string base_;
    std::ostream* out_;

    std::ostream* open();
    void emitAtts(const stream::Entry& node);
};

std::ostream*
Emitter::open()
{
  return &std::cout;
}

// Written so that '0' is written properly.
void
Emitter::putInt(int val)
{
  assert(val >= 0);
  if (val < 128)
    putChar(val);
  else {
    putChar(val & 127);
    putInt(val >> 7);
  }
}

void
Emitter::emitAtts(const stream::Entry& node)
{
  typedef stream::Entry::Atts::const_iterator iter;
  const stream::Entry::Atts& atts = node.getAtts();
  const iter end = atts.end();
  for (iter i = atts.begin(); i != end; ++i) {
    const string& key = i->first;
    const string& val = i->second;
    putChar('a');
    putString(key);
    putString(val);
  }
  putChar('A');
}

void
Emitter::walk(stream::DirEntryProxy root)
{
  const string name = root->getName();
  putChar('d');
  putString(name);

  // Dump my attributes.
  emitAtts(*root);

  // Walk subdirs.
  const stream::FsDirSourceProxy here = root->getDirSource();
  typedef stream::FsDirSource::DirIterator DI;
  const DI diEnd = here->dirEnd();
  for (DI i = here->dirBegin(); i != diEnd; ++i) {
    walk(*i);
  }

  // Walk the files.
  typedef stream::FsDirSource::FileIterator FI;
  const FI fiEnd = here->fileEnd();
  for (FI i = here->fileBegin(); i != fiEnd; ++i) {
    const string& name = (*i)->getName();
    putChar('f');
    putString(name);
    emitAtts(**i);
  }

  putChar('D');
}

void saveSurefile(std::string surePath, std::string tmpPath, stream::DirEntryProxy root)
{
  Emitter emit("0sure");
  emit.walk(root);
}

}
