// Reading entries from a directory.

#ifndef __DIR_H__
#define __DIR_H__

#include <string>
#include <vector>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
}

namespace asure {

  // A single entry in a directory.  Represented by an entry name, and
  // the 'stat' data for that node.
  class DirNode {
    public:
      std::string name;
      struct stat stat;

      DirNode(std::string& n) { name = n; }
      bool operator<(const DirNode& other) const {
	return name < other.name;
      }
  };

  // The contents of a directory.
  class Directory {
    public:
      Directory(std::string path);
      ~Directory();

      std::vector<DirNode*>& getDirs() { return dirs_; }
      std::vector<DirNode*>& getNonDirs() { return nondirs_; }
    private:
      std::vector<DirNode*> dirs_;
      std::vector<DirNode*> nondirs_;
  };

}

#endif
