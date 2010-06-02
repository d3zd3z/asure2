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

    void walk(tree::DirEntryProxy root);

    void putChar(char ch) { out_.put(ch); }
    // void putInt(int val);
    void putString(const string& str);
    void putHex(unsigned ch) {
      if (ch < 10)
        putChar(ch + '0');
      else
        putChar(ch - 10 + 'a');
    }

    // Cleanly close the emitter, rotating log files.
    void close();
  private:
    const string base_;
    ios::filtering_ostream& out_;
    void emitAtts(const tree::Entry& node);
};

void saveSurefile(std::string baseName, tree::DirEntryProxy root)
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

// Strings have some minimal quoting, and are terminated with a space.
void
Emitter::putString(const string& str)
{
  typedef string::const_iterator iter;
  const iter end = str.end();
  for (iter i = str.begin(); i != end; ++i) {
    if (*i != '=' && std::isgraph(*i)) {
      putChar(*i);
    } else {
      putChar('=');
      putHex((*i >> 4) & 0xF);
      putHex(*i & 0xF);
    }
  }
  putChar(' ');
}

void
Emitter::emitAtts(const tree::Entry& node)
{
  typedef tree::Entry::Atts::const_iterator iter;
  const tree::Entry::Atts& atts = node.getAtts();
  const iter end = atts.end();
  putChar('[');
  for (iter i = atts.begin(); i != end; ++i) {
    const string& key = i->first;
    const string& val = i->second;
    putString(key);
    putString(val);
  }
  putChar(']');
}

void
Emitter::walk(tree::DirEntryProxy root)
{
  const string name = root->getName();
  putChar('d');
  putString(name);

  // Dump my attributes.
  emitAtts(*root);
  putChar('\n');

  // Walk subdirs.
  typedef tree::DirEntry::dir_iterator DI;
  for (DI& i = root->dirIter(); !i.empty(); ++i) {
    walk(*i);
  }

  // Walk the files.
  typedef tree::DirEntry::file_iterator FI;
  for (FI& i = root->fileIter(); !i.empty(); ++i) {
    const string& name = (*i)->getName();
    putChar('f');
    putString(name);
    emitAtts(**i);
    putChar('\n');
  }

  putChar('u');
  putChar('\n');
}

}
