// Writing surefiles.

#ifndef __SUREFILE_H__
#define __SUREFILE_H__

#include <string>
#include "tree.hh"

namespace asure {

namespace extensions {
const std::string base = ".dat.gz";
const std::string tmp = ".0.gz";
const std::string bak = ".bak.gz";
}

void saveSurefile(std::string const& baseName, tree::NodeIterator& root);
tree::NodeIterator* loadSurefile(std::string const& fullName);

}

#endif
