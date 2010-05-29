// Reading entries from a directory.

#ifndef __DIR_H__
#define __DIR_H__

#include <string>
#include <vector>
#include <tr1/memory>

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
typedef std::tr1::shared_ptr<DirNode> DirNodeProxy;

// The contents of a directory.
class Directory {
  public:
    Directory(std::string path);
    ~Directory();

    const std::vector<DirNodeProxy>& getDirs() const { return dirs_; }
    const std::vector<DirNodeProxy>& getNonDirs() const { return nondirs_; }
  private:
    std::vector<DirNodeProxy> dirs_;
    std::vector<DirNodeProxy> nondirs_;
};

}

#endif
