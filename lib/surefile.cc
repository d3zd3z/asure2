// Writing surefiles

#include <cassert>
#include <fstream>
#include <iostream>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include "surefile.hh"
#include "exn.hh"

namespace asure {

using std::string;
namespace ios = boost::iostreams;

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

//////////////////////////////////////////////////////////////////////
// Reader of trees.

class SurefileIterator : public tree::NodeIterator {
 public:
  SurefileIterator() : file(), in(), depth(0), almostDone(false), done(false) { }
  void open(std::string const& path);
  bool empty() const { return done; }
  void operator++();
  tree::Node const& operator*() const { return node; }
 private:
  std::ifstream file;
  ios::filtering_istream in;

  class SubNode : public tree::Node {
   public:
    ~SubNode() { }
    Kind getKind() const { return kind; }
    std::string const& getName() const { return name; }
    Atts const& getAtts() const { return atts; }

    void clear() {
      name.clear();
      atts.clear();
    }

    Kind kind;
    std::string name;
    Atts atts;
  };
  SubNode node;
  int depth;
  bool almostDone, done;

  void parseError(char const* msg) {
    BOOST_THROW_EXCEPTION(parse_error() << message_code(msg));
  }

  void expect(char expCh) {
    char ch;
    in.get(ch);
    if (ch != expCh)
      parseError((std::string("Unexpected character: '") + ch + "', expecting '" + expCh + "'").c_str());
  }

  void readFull();
  void readString(std::string& name);
  char dehex(char ch);
};

void SurefileIterator::open(std::string const& path)
{
  file.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
  in.push(ios::gzip_decompressor());
  in.push(file);

  // Read in the header.
  int const len = surefileMagic.length();
  char rawHeader[len];
  in.read(rawHeader, len);
  string const magic(rawHeader, len);

  if (magic != surefileMagic)
    parseError("Invalid file header");

  // Advance to the first entity.
  operator++();
}

void SurefileIterator::operator++()
{
  if (almostDone) {
    done = true;
    return;
  }

  char code;
  in.get(code);

  node.clear();
  switch (code) {
    case 'd':
      node.kind = tree::Node::ENTER;
      readFull();
      ++depth;
      break;
    case 'f':
      node.kind = tree::Node::NODE;
      readFull();
      break;
    case '-':
      node.kind = tree::Node::MARK;
      break;
    case 'u':
      node.kind = tree::Node::LEAVE;
      --depth;
      if (depth == 0)
        almostDone = true;
      break;
    default:
      parseError((std::string("Unknown code: '") + code + '\'').c_str());
  }
  expect('\n');
}

void SurefileIterator::readFull()
{
  readString(node.name);
  expect('[');

  while (true) {
    char ch;
    in.get(ch);
    if (ch == ']')
      break;
    std::string key(1, ch);
    readString(key);
    std::string val;
    readString(val);
    node.atts[key] = val;
  }
}

// Read a space-terminated name, appending to the 'name'.
void SurefileIterator::readString(std::string& name)
{
  while (true) {
    char ch;
    in.get(ch);
    if (ch == ' ')
      break;
    else if (ch == '=') {
      char a, b;
      in.get(a);
      in.get(b);
      name += (dehex(a) << 4) | (dehex(b));
    } else
      name += ch;
  }
}

char SurefileIterator::dehex(char ch)
{
  if (ch >= '0' && ch <= '9')
    return ch - '0';
  else if (ch >= 'a' && ch <= 'f')
    return ch - 'a' + 10;
  else
    parseError("Invalid hex character");
  abort();
}

tree::NodeIterator* loadSurefile(std::string const& fullName)
{
  std::auto_ptr<SurefileIterator> tree(new SurefileIterator());

  tree->open(fullName);

  return tree.release();
}

}
