// Tree.

#include "tree.hh"

namespace asure {
namespace tree {

NodeIterator::~NodeIterator()
{
}

Node::~Node()
{
}

Node::Atts Node::getFullAtts() const
{
  Atts result = this->getExpensiveAtts();
  Atts const& cheapAtts = this->getAtts();
  result.insert(cheapAtts.begin(), cheapAtts.end());
  return result;
}

std::string const Node::emptyName = "";
Node::Atts const Node::emptyAtts = Atts();

}
}
