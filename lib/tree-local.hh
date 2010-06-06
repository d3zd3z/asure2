// Local directory trees.

#ifndef __TREE_LOCAL_H__
#define __TREE_LOCAL_H__

#include <list>
#include <string>
#include "tree.hh"

namespace asure {
namespace tree {

// Return a newly allocated NodeIterator that traverses a directory in the
// filesystem.  The iterator should be returned with delete when finished.
NodeIterator* walkTree(std::string const& path);

}
}

#endif
