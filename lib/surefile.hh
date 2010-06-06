// Writing surefiles.

#ifndef __SUREFILE_H__
#define __SUREFILE_H__

#include <string>
#include "tree.hh"

namespace asure {

void saveSurefile(std::string const& baseName, tree::NodeIterator& root);

}

#endif
