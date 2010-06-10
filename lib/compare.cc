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

class Comparer {
 public:
  Comparer(tree::NodeIterator& left_, tree::NodeIterator& right_) :
      left(left_), right(right_), path()
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
  tree::NodeIterator& left;
  tree::NodeIterator& right;
  std::stack<std::string> path;

  void compareAtts();

  void skipLeft();
  void skipRight();

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

void Comparer::skipRight()
{
  std::cout << "+ dir                    " << getPath() << '/' << rightName() << '\n';
  skipTree(right);
}

}

void compareTrees(tree::NodeIterator& oldTree, tree::NodeIterator& newTree)
{
  assert(oldTree->getKind() == Node::ENTER);
  assert(newTree->getKind() == Node::ENTER);

  Comparer comp(oldTree, newTree);
  comp.dir();
}

}
