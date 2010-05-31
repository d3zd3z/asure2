// Writing surefiles

#include <cassert>
#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "surefile.hh"

namespace asure {

using std::string;
namespace ios = boost::iostreams;

namespace extensions {
const string base = ".dat.gz";
const string tmp = ".0.gz";
const string bak = ".bak.gz";
}

const string surefileMagic = "asure-2.0\n-----\n";

class Emitter {
  public:
    Emitter(const string& base, ios::filtering_ostream& out) : base_(base), out_(out) { }
    ~Emitter();

    void walk(stream::DirEntryProxy root);

    void putChar(char ch) { out_.put(ch); }
    void putInt(int val);
    void putString(const string& str) {
      putInt(str.length());
      out_.write(str.data(), str.length());
    }

    // Cleanly close the emitter, rotating log files.
    void close();
  private:
    const string base_;
    ios::filtering_ostream& out_;
    void emitAtts(const stream::Entry& node);
};

void saveSurefile(std::string baseName, stream::DirEntryProxy root)
{
  string tmpName = baseName + extensions::tmp;

  std::ofstream file(tmpName.c_str(), std::ios_base::out | std::ios_base::binary);
  ios::filtering_ostream out;

  out.push(ios::gzip_compressor());
  out.push(file);

  out.write(surefileMagic.data(), surefileMagic.length());

  Emitter emit(baseName, out);
  emit.walk(root);
  emit.close();
}

// Upon clean close, close up the stream, and rotate the files.
void
Emitter::close()
{
  out_.reset();
  std::rename((base_ + extensions::base).c_str(), (base_ + extensions::bak).c_str());
  std::rename((base_ + extensions::tmp).c_str(), (base_ + extensions::base).c_str());
}

Emitter::~Emitter()
{
  // If we didn't close properly unlink the temp file.
  if (!out_.empty()) {
    out_.reset();
    unlink((base_ + extensions::tmp).c_str());
  }
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

}
