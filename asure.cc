/* File integrity checking.
 */

#include <iostream>
#include <string>
#include <vector>

#include "dir.h"
#include "hash.h"

using std::string;
using std::vector;
using std::cout;

using asure::Hash;
using asure::DirNode;
using asure::Directory;

static void indent(int level)
{
  for (int i = 0; i < level; i++)
    cout << " ";
}

void walk(string path, string name, int level)
{
  Directory here(path);
  typedef vector<DirNode*>::iterator iter;
  indent(level);
  cout << "Enter " << name << '\n';
  {
    vector<DirNode*>& dirs = here.getDirs();
    for (iter i = dirs.begin(); i != dirs.end(); i++) {
      walk(path + '/' + (*i)->name, (*i)->name, level + 1);
    }
  }
  {
    vector<DirNode*>& nondirs = here.getNonDirs();
    for (iter i = nondirs.begin(); i != nondirs.end(); i++) {
      indent(level+1);
      cout << "Item " << (*i)->name << '\n';
      if (S_ISREG((*i)->stat.st_mode)) {
	Hash hash;
	hash.ofFile(path + "/" + (*i)->name);
	indent(level);
	cout << "  hash: " << static_cast<string>(hash) << '\n';
      }
    }
  }
  indent(level);
  cout << "Leave " << name << '\n';
}

int main()
{
  try {
    walk(".", "__root__", 0);
  }
  catch (int ret) {
    cout << "Raised: " << ret << '\n';
  }
}
