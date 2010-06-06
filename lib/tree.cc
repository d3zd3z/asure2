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

std::string const Node::emptyName = "";
Node::Atts const Node::emptyAtts = Atts();

}
}
