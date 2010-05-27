// Directories.

extern "C" {
#include <dirent.h>
#include <errno.h>
}

#include <string>
#include <vector>
#include <algorithm>

#include "dir.h"

using std::string;
using std::vector;

namespace asure {

class NameIno {
  public:
    string name;
    ino_t ino;

    NameIno(string& n, ino_t i) { name = n; ino = i; }
    bool operator<(const NameIno& other) const {
      return ino < other.ino;
    }

    // Read all of the name/inode pairs from a directory, sorted by
    // inode number.
    static void getNames(string path, vector<NameIno>& result);
};

Directory::Directory(string path)
{
  vector<NameIno> names;
  NameIno::getNames(path, names);

  // Stat all of the names, in inode order.
  std::sort(names.begin(), names.end());
  for (vector<NameIno>::iterator i = names.begin(); i != names.end(); i++) {
    DirNode* node = new DirNode(i->name);
    int result = lstat((path + "/" + i->name).c_str(), &node->stat);
    if (result == 0) {
      if (S_ISDIR(node->stat.st_mode))
	dirs_.push_back(node);
      else
	nondirs_.push_back(node);
    } else {
      // TODO: Warn about unable to stat.
      delete node;
    }
  }

  // Sort the results by name.
  std::sort(dirs_.begin(), dirs_.end());
  std::sort(nondirs_.begin(), nondirs_.end());
}

Directory::~Directory()
{
  typedef vector<DirNode*>::iterator iter;
  for (iter i = dirs_.begin(); i != dirs_.end(); i++)
    delete *i;
  for (iter i = nondirs_.begin(); i != nondirs_.end(); i++)
    delete *i;
}

// Close an open directory.
class DirCloser {
  public:
    DirCloser(DIR* p) { dir = p; }
    ~DirCloser() {
      int r = closedir(dir);
      if (r != 0)
	// TODO: What to do?
	throw errno;
    }

  private:
    DIR* dir;
};


void NameIno::getNames(string path, vector<NameIno>& result)
{
  DIR *dirp = opendir(path.c_str());
  if (dirp == NULL)
    throw errno;
  DirCloser cleanup(dirp);

  while (true) {
    errno = 0;
    struct dirent* ent = readdir(dirp);
    if (ent == 0 && errno == 0)
      break;
    if (ent == 0)
      throw errno;
    string name = string(ent->d_name);

    if (name != "." && name != "..")
      result.push_back(NameIno(name, ent->d_ino));
  }
}

}