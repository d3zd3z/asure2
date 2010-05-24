/* File integrity checking.
 */

#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <vector>

using std::string;
using std::printf;
using std::vector;

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
}

class dirnode {
  typedef std::pair<string, struct stat *> node_t;
  vector<node_t> dirs;
  vector<node_t> nondirs;
  public:
    dirnode(string path);
    ~dirnode();
};

dirnode::dirnode(string path)
{
  DIR *dirp = opendir(path.c_str());
  if (dirp == NULL)
    throw 5;

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

    struct stat *sbuf = new struct stat;
    int result = lstat((path + "/" + name).c_str(), sbuf);
    if (result != 0)
      throw 8;

    const node_t &node = node_t(name, sbuf);
    if (S_ISDIR(sbuf->st_mode))
      dirs.push_back(node);
    else
      nondirs.push_back(node);
    // std::auto_ptr<struct stat> buf (new struct stat);
    // std::cout << buf << '\n';
    // std::cout << name << '\n';
  }

  // TODO: Make this cleanuppable.
  int result = closedir(dirp);
  if (result != 0)
    throw 7;
}

dirnode::~dirnode()
{
  typedef vector<node_t>::iterator iter;
  for (iter i = dirs.begin(); i != dirs.end(); i++)
    delete i->second;
  for (iter i = nondirs.begin(); i != nondirs.end(); i++)
    delete i->second;
}

int main()
{
  dirnode root(".");
}
