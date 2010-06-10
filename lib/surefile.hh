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

class Emitter;

// The surefile can be written either 'push' style, or the 'save' method used to
// pull from a NodeIterator.
class SurefileSaver {
 public:
  SurefileSaver(std::string const& baseName);
  ~SurefileSaver();

  void writeNode(tree::Node const& node);

  // Cleanly finish writing the file.  If close() is not called, then the
  // surefile will be partially written.
  void close();

  // Save the surefile.
  static void save(std::string const& baseName, tree::NodeIterator& root);

 private:
  Emitter* emit;
};

tree::NodeIterator* loadSurefile(std::string const& fullName);

}

#endif
