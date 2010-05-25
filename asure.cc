/* File integrity checking.
 */

#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <vector>
#include <algorithm>

#include "dir.h"
#include "hash.h"

using std::string;
using std::printf;
using std::vector;
using std::cout;
using std::ios_base;

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
}

class dirnode {
  public:
  struct node_t {
    string name;
    struct stat *stat;
    node_t(string& a, struct stat *b) {
      name = a;
      stat = b;
    }

    bool operator<(const node_t& other) const {
      return name < other.name;
    }
  };
  vector<node_t> dirs;
  vector<node_t> nondirs;
  dirnode(string path);
  ~dirnode();
};

struct name_ino_t {
  string name;
  ino_t ino;
  name_ino_t(string& n, ino_t& i) { name = n; ino = i; }
  bool operator<(const name_ino_t& other) const {
    return ino < other.ino;
  }
};

class dircloser {
  DIR *dirp;
  public:
    dircloser(DIR *p) { dirp = p; }
    ~dircloser() {
      cout << "closing\n";
      int r = closedir(dirp);
      if (r != 0)
	// TODO: Something meaningful to do here?
	throw 7;
    }
};

// Read in all of the names in a given directory, pairing them up with their
// inode number.
static void
get_names(string path, vector<name_ino_t>& result)
{
  DIR *dirp = opendir(path.c_str());
  if (dirp == NULL)
    throw 5;
  dircloser cleanup(dirp);

  while (true) {
    errno = 0;
    struct dirent *ent = readdir(dirp);
    if (ent == NULL && errno == 0)
      break;
    if (ent == NULL)
      throw 6;
    string name = ent->d_name;
    if (name == "." || name == "..")
      continue;

    result.push_back(name_ino_t(name, ent->d_ino));
  }
}

dirnode::dirnode(string path)
{
  vector<name_ino_t> names;
  get_names(path, names);

  // Stat all of the names, in inode order.
  // std::sort(names.begin(), names.end(), ino_lt);
  std::sort(names.begin(), names.end());
  for (vector<name_ino_t>::iterator i = names.begin(); i != names.end(); i++) {
    struct stat *sbuf = new struct stat;
    int result = lstat((path + "/" + i->name).c_str(), sbuf);
    if (result == 0) {
      const node_t &node = node_t(i->name, sbuf);
      if (S_ISDIR(sbuf->st_mode))
        dirs.push_back(node);
      else
        nondirs.push_back(node);
    } else {
      // TODO: Warn about unable to stat.
      delete sbuf;
    }
  }

  // Sort the results by name.
  std::sort(dirs.begin(), dirs.end());
  std::sort(nondirs.begin(), nondirs.end());
}

dirnode::~dirnode()
{
  typedef vector<node_t>::iterator iter;
  for (iter i = dirs.begin(); i != dirs.end(); i++)
    delete i->stat;
  for (iter i = nondirs.begin(); i != nondirs.end(); i++)
    delete i->stat;
}

void walk(string path, string name)
{
  asure::Directory here(path);
  /*
  typedef vector<dirnode::node_t>::iterator iter;
  cout << "Enter " << name << "\n";
  for (iter i = here.dirs.begin(); i != here.dirs.end(); i++) {
    walk(path + '/' + i->name, i->name);
  }
  for (iter i = here.nondirs.begin(); i != here.nondirs.end(); i++) {
    cout << "Item  " << i->name << '\n';
    if (S_ISREG(i->stat->st_mode)) {
      file_hash hash;
      hash_file(path + "/" + i->name, hash);
      cout << "  hash: " << static_cast<string>(hash) << '\n';
    }
  }
  cout << "Leave " << name << "\n";
  */
}

int main()
{
  try {
    walk(".", "__root__");
  }
  catch (int ret) {
    cout << "Raised: " << ret << '\n';
  }
}
