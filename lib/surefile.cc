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

    void putRegular(char code, tree::Node const& node);

    void putChar(char ch) { out_.put(ch); }
    void putSimple(char code) {
      putChar(code);
      putChar('\n');
    }
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
    void emitAtts(tree::Node const& node);
};

void saveSurefile(std::string const& baseName, tree::NodeIterator& root)
{
  string tmpName = baseName + extensions::tmp;

  std::ofstream file(tmpName.c_str(), std::ios_base::out | std::ios_base::binary);
  ios::filtering_ostream out;

  out.push(ios::gzip_compressor());
  out.push(file);

  out.write(surefileMagic.data(), surefileMagic.length());

  Emitter emit(baseName, out);
  for (; !root.empty(); ++root) {
    tree::Node const& node = *root;

    switch (node.getKind()) {
      case tree::Node::ENTER:
        emit.putRegular('d', node);
        break;
      case tree::Node::NODE:
        emit.putRegular('f', node);
        break;
      case tree::Node::MARK:
        emit.putSimple('-');
        break;
      case tree::Node::LEAVE:
        emit.putSimple('u');
    }
  }

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

void
Emitter::putRegular(char code, tree::Node const& node)
{
  putChar(code);
  putString(node.getName());
  emitAtts(node);
  putChar('\n');
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
Emitter::emitAtts(tree::Node const& node)
{
  tree::Node::Atts atts = node.getExpensiveAtts();
  tree::Node::Atts const& mainAtts = node.getAtts();
  atts.insert(mainAtts.begin(), mainAtts.end());

  typedef tree::Node::Atts::const_iterator Iter;
  Iter const end = atts.end();
  putChar('[');
  for (Iter i = atts.begin(); i != end; ++i) {
    putString(i->first);
    putString(i->second);
  }
  putChar(']');
}

}
