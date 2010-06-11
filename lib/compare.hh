// Comparing two trees.

#ifndef __LIB_COMPARE_H__
#define __LIB_COMPARE_H__

#include "tree.hh"
#include "surefile.hh"

namespace asure {

void compareTrees(tree::NodeIterator& oldTree, tree::NodeIterator& newTree);
void updateTree(tree::NodeIterator& oldTree, tree::NodeIterator& newTree, SurefileSaver& saver);

}

#endif
