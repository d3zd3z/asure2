// Comparing two trees.

#include <cassert>
#include <map>
#include <stack>
#include <iostream>
#include <vector>
#include "compare.hh"

namespace asure {

namespace {

using tree::Node;

// Implementation class to help with the combining of two trees.
class Combiner {
 public:
  Combiner(tree::NodeIterator& left_, tree::NodeIterator& right_) :
      left(left_), right(right_)
  {
    assert(left->getKind() == Node::ENTER);
    assert(right->getKind() == Node::ENTER);
  }

 protected:
  tree::NodeIterator& left;
  tree::NodeIterator& right;

  // Utility operations.
  bool isLeftMark() { return left->getKind() == Node::MARK; }
  bool isLeftEnter() { return left->getKind() == Node::ENTER; }
  bool isLeftLeave() { return left->getKind() == Node::LEAVE; }
  bool isLeftNode() { return left->getKind() == Node::NODE; }
  bool isRightMark() { return right->getKind() == Node::MARK; }
  bool isRightEnter() { return right->getKind() == Node::ENTER; }
  bool isRightLeave() { return right->getKind() == Node::LEAVE; }
  bool isRightNode() { return right->getKind() == Node::NODE; }

  std::string const& leftName() { return left->getName(); }
  std::string const& rightName() { return right->getName(); }
};

class Comparer : Combiner {
 public:
  Comparer(tree::NodeIterator& left_, tree::NodeIterator& right_) :
      Combiner(left_, right_), path()
  {
    push(".");
  }

  void dir();

  void push(std::string const& name) {
    if (path.empty())
      path.push(name);
    else
      path.push(path.top() + '/' + name);
  }
  void pop() {
    path.pop();
  }
  std::string& getPath() { return path.top(); }
 private:
  std::stack<std::string> path;

  void compareAtts();

  void skipLeft();
  void skipRight();
};

class Updater : Combiner {
 public:
  Updater(tree::NodeIterator& left_, tree::NodeIterator& right_,
          SurefileSaver& saver_) :
      Combiner(left_, right_), saver(saver_) { }

  void dir();

 private:
  SurefileSaver& saver;

  void skipLeft();
  void storeRight();
  bool sameAtt(std::string const& aname);
};

void Comparer::compareAtts()
{
  Node::Atts latts = left->getFullAtts();
  Node::Atts ratts = right->getFullAtts();

  latts.erase("ctime");
  ratts.erase("ctime");
  latts.erase("ino");
  ratts.erase("ino");

  std::vector<std::string> diffs;
  typedef Node::Atts::const_iterator Iter;

  Iter const lend = latts.end();
  Iter const rend = ratts.end();

  Iter lpos = latts.begin();
  Iter rpos = ratts.begin();

  while (lpos != lend && rpos != rend) {
    if (rpos == rend || (lpos != lend && lpos->first < rpos->first)) {
      std::cout << "Missing attribute: " << lpos->first << '\n';
      ++lpos;
    } else if (lpos == lend || (lpos->first > rpos->first)) {
      std::cout << "Extra attribute: " << rpos->first << '\n';
      ++rpos;
    } else {
      if (lpos->second != rpos->second) {
        diffs.push_back(lpos->first);
      }
      ++lpos;
      ++rpos;
    }
  }

  if (!diffs.empty()) {
    int len = 0;
    std::cout << "  [";

    typedef std::vector<std::string>::const_iterator DI;
    DI const begin = diffs.begin();
    DI const end = diffs.end();
    for (DI i = begin; i != end; ++i) {
      if (i != begin) {
        ++len;
        std::cout << ',';
      }
      std::cout << *i;
      len += i->length();
    }
    for (; len < 20; ++len) {
      std::cout << ' ';
    }
    std::cout << "] " << getPath() << '\n';
  }
}

// Called when comparing two directories, where the names match.
void Comparer::dir()
{
  compareAtts();

  // std::cout << "Comparing: " << getPath() << '\n';
  assert(isLeftEnter());
  assert(isRightEnter());
  assert(leftName() == rightName());

  ++left;
  ++right;

  // Walk through the subdirectories of both nodes.
  while (isLeftEnter() || isRightEnter()) {
    assert(!isLeftLeave());
    assert(!isRightLeave());
    if (!isRightEnter() || (isLeftEnter() && leftName() < rightName())) {
      skipLeft();
    } else if (!isLeftEnter() || leftName() > rightName()) {
      skipRight();
    } else {
      // Both on the same dir.
      push(leftName());
      dir();
      pop();
    }
  }

  // Both nodes are sitting on mark.
  assert(isLeftMark());
  assert(isRightMark());
  ++left;
  ++right;

  // Compare the files themselves.
  while (isLeftNode() || isRightNode()) {
    assert(!isLeftEnter());
    assert(!isRightEnter());
    if (!isRightNode() || (isLeftNode() && leftName() < rightName())) {
      std::cout << "- file                   " << getPath() << '/' << leftName() << '\n';
      ++left;
    } else if (!isLeftNode() || leftName() > rightName()) {
      std::cout << "+ file                   " << getPath() << '/' << rightName() << '\n';
      ++right;
    } else {
      push(leftName());
      compareAtts();
      pop();
      ++left;
      ++right;
    }
  }

  assert(isLeftLeave());
  assert(isRightLeave());
  ++left;
  ++right;
}

// A node referencing another node, with augmented attributes.
class AttNode : public Node {
 public:
  AttNode(Node const& other_, Node::Atts const& fullAtts_) :
      other(other_), fullAtts(fullAtts_) { }
  ~AttNode() { }

  Kind getKind() const { return other.getKind(); }
  std::string const& getName() const { return other.getName(); }
  Atts const& getAtts() const { return fullAtts; }

 private:
  Node const& other;
  Node::Atts const& fullAtts;
};

// Called to update a directory.
void Updater::dir()
{
  assert(isLeftEnter());
  assert(isRightEnter());
  assert(leftName() == rightName());

  saver.writeNode(*right);

  ++left;
  ++right;

  // Walk through the subdirectories of both nodes.
  while (isLeftEnter() || isRightEnter()) {
    assert(!isLeftLeave());
    assert(!isRightLeave());
    if (!isRightEnter() || (isLeftEnter() && leftName() < rightName())) {
      // The left can just be skipped.
      skipLeft();
    } else if (!isLeftEnter() || leftName() > rightName()) {
      // A new directory is just written out.
      storeRight();
    } else {
      // Both on the same dir, recurse.
      dir();
    }
  }

  // Both nodes are sitting on mark.
  assert(isLeftMark());
  assert(isRightMark());
  saver.writeNode(*right);
  ++left;
  ++right;

  // Compare the files themselves.
  while (isLeftNode() || isRightNode()) {
    assert(!isLeftEnter());
    assert(!isRightEnter());
    if (!isRightNode() || (isLeftNode() && leftName() < rightName())) {
      // Removed file, nothing to do.
      ++left;
    } else if (!isLeftNode() || leftName() > rightName()) {
      // Added file, write out full node information.
      saver.writeNode(*right);
      ++right;
    } else {
      // Write 'right' node, possibly using new atts.
      if (sameAtt("ino") && sameAtt("ctime")) {
        Node::Atts fullAtts = left->getExpensiveAtts();
        Node::Atts const& mainAtts = left->getAtts();
        fullAtts.insert(mainAtts.begin(), mainAtts.end());

        AttNode tmp(*right, fullAtts);
        saver.writeNode(tmp);
      } else {
        saver.writeNode(*right);
      }
      ++left;
      ++right;
    }
  }

  assert(isLeftLeave());
  assert(isRightLeave());
  saver.writeNode(*right);
  ++left;
  ++right;
}

// Skip a tree, assuming we're sitting on the ENTER node for it.
void skipTree(tree::NodeIterator& tree)
{
  ++tree;
  int depth = 1;
  while (depth > 0) {
    if (tree->getKind() == Node::ENTER)
      ++depth;
    else if (tree->getKind() == Node::LEAVE)
      --depth;
    ++tree;
  }
}

void Comparer::skipLeft()
{
  std::cout << "- dir                    " << getPath() << '/' << leftName() << '\n';
  skipTree(left);
}

void Updater::skipLeft()
{
  skipTree(left);
}

void Comparer::skipRight()
{
  std::cout << "+ dir                    " << getPath() << '/' << rightName() << '\n';
  skipTree(right);
}

void Updater::storeRight()
{
  saver.writeNode(*right);
  ++right;
  int depth = 1;
  while (depth > 0) {
    if (right->getKind() == Node::ENTER)
      ++depth;
    else if (right->getKind() == Node::LEAVE)
      --depth;
    saver.writeNode(*right);
    ++right;
  }
}

// Return true if both nodes have the named att, and the att has the same value.
bool Updater::sameAtt(std::string const& aname)
{
  typedef Node::Atts::const_iterator Iter;
  Node::Atts const& latts = left->getAtts();
  Iter const l = latts.find(aname);
  if (l == latts.end())
    return false;
  Node::Atts const& ratts = right->getAtts();
  Iter const r = ratts.find(aname);
  if (r == ratts.end())
    return false;

  return l->second == r->second;
}

}

void compareTrees(tree::NodeIterator& oldTree, tree::NodeIterator& newTree)
{
  Comparer comp(oldTree, newTree);
  comp.dir();
}

void updateTree(tree::NodeIterator& oldTree, tree::NodeIterator& newTree, SurefileSaver& saver)
{
  Updater update(oldTree, newTree, saver);
  update.dir();
  saver.close();
}

}
