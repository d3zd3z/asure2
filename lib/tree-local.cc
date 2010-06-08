// Local directory trees.
//
// TODO: Avoid crossing device boundaries.

extern "C" {
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <algorithm>
#include <iostream>
#include <sstream>
#include <deque>
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>

#include "hash.hh"
#include "tree-local.hh"
#include "exn.hh"

namespace asure {
namespace tree {

namespace {

class NodeWrapper;
typedef std::deque<NodeWrapper*> NodeDeque;

class NodeWrapper : boost::noncopyable {
 public:
  virtual ~NodeWrapper() = 0;
  virtual Node const& getNode() const = 0;

  // Advance is called _after_ removing 'this' from the deque.  When this method
  // returns, 'this' will be deleted.
  virtual void advance(NodeDeque& /*dirs*/) { }
};

class Tree : public NodeIterator {
 public:
  ~Tree();

  bool empty() const { return nodes.empty(); }
  void operator++();
  Node const& operator*() const {
    return nodes.front()->getNode();
  }

  NodeDeque nodes;
};

Tree::~Tree()
{
  // Clean up.  Note that this doesn't call advance.
  while (!nodes.empty()) {
    delete nodes.front();
    nodes.pop_front();
  }
}

void Tree::operator++()
{
  NodeWrapper* head = nodes.front();
  nodes.pop_front();
  head->advance(nodes);
  delete head;
}

NodeWrapper::~NodeWrapper()
{
}

struct NameIno {
 public:
  NameIno(std::string& n, ino_t i) : name(n), ino(i) { }
  bool operator<(const NameIno& other) const {
    return ino < other.ino;
  }

  // Read all of the name/inode pairs from a directory, sorted by inode number.
  static void getNames(const std::string& path, std::vector<NameIno>& result);

  std::string name;
  ino_t ino;

};

class SimpleNodeWrapper : public NodeWrapper {
 public:
  SimpleNodeWrapper(Node::Kind kind) : node_(kind) { }
  Node const& getNode() const { return node_; }
 private:
  class SubNode : public Node {
   public:
    SubNode(Kind kind) : kind_(kind) { }
    Kind getKind() const { return kind_; }
    std::string const& getName() const { return Node::emptyName; }
    Atts const& getAtts() const { return Node::emptyAtts; }
   private:
    Kind kind_;
  };
  SubNode node_;
};

class RegularNodeWrapper : public NodeWrapper {
 public:
  RegularNodeWrapper(std::string const& name, std::string const& path, struct stat& stat);

  Node const& getNode() const { return node_; }

 private:
  class SubNode : public Node {
   public:
    SubNode(std::string const& name, std::string const& path) : name_(name), path_(path), atts_() { }
    Kind getKind() const { return Node::NODE; }
    std::string const& getName() const { return name_; }
    Atts const& getAtts() const { return atts_; }
    Atts getExpensiveAtts() const;

    std::string name_;
    std::string path_;
    Node::Atts atts_;
  };
  SubNode node_;
};

// Directory iteration.
class DirNodeWrapper : public NodeWrapper {
 public:
  DirNodeWrapper(std::string const& name, std::string const& path, struct stat& stat);

  Node const& getNode() const { return node_; }
  void advance(NodeDeque& dirs);

 private:
  class SubNode : public Node {
   public:
    SubNode(std::string const& name) : name_(name), atts_() { }
    Kind getKind() const { return Node::ENTER; }
    std::string const& getName() const { return name_; }
    Atts const& getAtts() const { return atts_; }

    std::string name_;
    Node::Atts atts_;
  };
  SubNode node_;
  std::string path_;
};

template <class N>
std::string stringify(N value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

template <class E>
bool nameLess(E const& a, E const& b)
{
  return a->getName() < b->getName();
}

// The C api is very weird.
std::string getLink(std::string const& path, int length)
{
  char buf[length];
  ssize_t len = readlink(path.c_str(), buf, length);
  if (len < 0)
    throw std::exception();
  else if (len < length)
    return std::string(buf, len);
  else
    return getLink(path, 2*length);
}

bool wrapperLess(NodeWrapper const* a, NodeWrapper const* b)
{
  return a->getNode().getName() < b->getNode().getName();
}

DirNodeWrapper::DirNodeWrapper(std::string const& name, std::string const& path, struct stat& stat) :
    node_(name), path_(path)
{
  node_.atts_["kind"] = "dir";
  node_.atts_["uid"] = stringify(stat.st_uid);
  node_.atts_["gid"] = stringify(stat.st_gid);
  node_.atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
}

RegularNodeWrapper::RegularNodeWrapper(std::string const& name, std::string const& path, struct stat& stat) :
    node_(name, path)
{
  Node::Atts& atts = node_.atts_;

  if (S_ISREG(stat.st_mode)) {
    atts["kind"] = "file";
    atts["uid"] = stringify(stat.st_uid);
    atts["gid"] = stringify(stat.st_gid);
    atts["mtime"] = stringify(stat.st_mtime);
    atts["ctime"] = stringify(stat.st_ctime);
    atts["ino"] = stringify(stat.st_ino);
    atts["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISLNK(stat.st_mode)) {
    atts["kind"] = "lnk";
    std::string const target = getLink(path, 128);
    atts["targ"] = target;
  } else if (S_ISSOCK(stat.st_mode)) {
    atts["kind"] = "sock";
    atts["uid"] = stringify(stat.st_uid);
    atts["gid"] = stringify(stat.st_gid);
    atts["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISFIFO(stat.st_mode)) {
    atts["kind"] = "fifo";
    atts["uid"] = stringify(stat.st_uid);
    atts["gid"] = stringify(stat.st_gid);
    atts["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISBLK(stat.st_mode)) {
    atts["kind"] = "blk";
    atts["uid"] = stringify(stat.st_uid);
    atts["gid"] = stringify(stat.st_gid);
    atts["perm"] = stringify(stat.st_mode & ~S_IFMT);
    atts["devmaj"] = stringify(major(stat.st_rdev));
    atts["devmin"] = stringify(minor(stat.st_rdev));
  } else if (S_ISCHR(stat.st_mode)) {
    atts["kind"] = "chr";
    atts["uid"] = stringify(stat.st_uid);
    atts["gid"] = stringify(stat.st_gid);
    atts["perm"] = stringify(stat.st_mode & ~S_IFMT);
    atts["devmaj"] = stringify(major(stat.st_rdev));
    atts["devmin"] = stringify(minor(stat.st_rdev));
  } else {
    throw std::exception();
  }
}

Node::Atts
RegularNodeWrapper::SubNode::getExpensiveAtts() const
{
  Atts atts;

  Atts::const_iterator kind = atts_.find("kind");
  assert(kind != atts_.end());
  if (kind->second == "file") {
    Hash h;
    h.ofFile(path_);
    atts["sha1"] = h;
  }

  return atts;
}

void DirNodeWrapper::advance(NodeDeque& dirs)
{
  dirs.push_front(new SimpleNodeWrapper(Node::LEAVE));

  std::vector<NodeWrapper*> subdirs;
  std::vector<NodeWrapper*> files;

  try {
    std::vector<NameIno> names;
    NameIno::getNames(path_, names);

    // Iterate through the entries, adding them appropraitely as a file or dir.
    typedef std::vector<NameIno>::const_iterator iter;
    iter const end = names.end();
    for (iter i = names.begin(); i != end; ++i) {
      struct stat stat;
      std::string const fullName = path_ + '/' + i->name;
      int result = lstat(fullName.c_str(), &stat);
      if (result != 0) {
        // TODO: Warn
        continue;
      }
      if (S_ISDIR(stat.st_mode)) {
        subdirs.push_back(new DirNodeWrapper(i->name, fullName, stat));
      } else {
        files.push_back(new RegularNodeWrapper(i->name, fullName, stat));
      }
    }
  }
  catch (io_error& e) {
    std::cout << "warning:\n-----\n" << boost::diagnostic_information(e) << "-----\n";
  }

  typedef std::vector<NodeWrapper*>::const_reverse_iterator riter;

  sort(subdirs.begin(), subdirs.end(), wrapperLess);
  sort(files.begin(), files.end(), wrapperLess);

  for (riter i = files.rbegin(); i != files.rend(); ++i) {
    dirs.push_front(*i);
  }

  dirs.push_front(new SimpleNodeWrapper(Node::MARK));

  for (riter i = subdirs.rbegin(); i != subdirs.rend(); ++i) {
    dirs.push_front(*i);
  }
}

}

NodeIterator* walkTree(std::string const& path)
{
  Tree* tree = new Tree;

  struct stat rootStat;
  int result = lstat(path.c_str(), &rootStat);
  if (result != 0)
    BOOST_THROW_EXCEPTION(io_error() << path_code(path) << errno_code(errno));
  if (!S_ISDIR(rootStat.st_mode))
    BOOST_THROW_EXCEPTION(io_error() << path_code(path));

  NodeWrapper* root = new DirNodeWrapper("__root__", path, rootStat);
  tree->nodes.push_front(root);

  return tree;
}

class DirCloser {
 public:
  DirCloser(DIR* dir) : dir_(dir) { }
  ~DirCloser() {
    closedir(dir_);
    // TODO: Log error if there is a problem.
  }

 private:
  DIR* dir_;

  DirCloser(DirCloser const& other);
  void operator=(DirCloser const& other);
};

void NameIno::getNames(const std::string& path, std::vector<NameIno>& result)
{
  DIR* dirp = opendir(path.c_str());
  if (dirp == NULL) {
    BOOST_THROW_EXCEPTION(io_error() << errno_code(errno) << path_code(path));
  }
  DirCloser cleanup(dirp);

  while (true) {
    errno = 0;
    struct dirent* ent = readdir(dirp);
    if (ent == 0 && errno == 0)
      break;
    if (ent == 0)
      throw std::exception();
    std::string name(ent->d_name);

    if (name == "." || name == "..")
      continue;

    // Skip over integrity files.
    if (name.find("0sure.") == 0)
      continue;
    if (name.find("2sure.") == 0)
      continue;

    result.push_back(NameIno(name, ent->d_ino));
  }

  std::sort(result.begin(), result.end());
}

}
}
