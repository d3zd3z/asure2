// Local directory trees.
//
// TODO: Avoid crossing device boundaries.

extern "C" {
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
}

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "hash.hh"
#include "tree-local.hh"

namespace asure {
namespace tree {

template <class N>
static std::string stringify(N value)
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

LocalDirEntry::OldDirEntry(std::string const& name, std::string const& path,
                           struct stat& stat) :
    Entry(name, path),
    computed_(false),
    files_(), dirs_()
{
  atts_["kind"] = "dir";
  atts_["uid"] = stringify(stat.st_uid);
  atts_["gid"] = stringify(stat.st_gid);
  atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
}

class LocalEntry : public Entry {
 public:
  LocalEntry(std::string const& name, std::string const& path,
             struct stat& stat);
  ~LocalEntry() { }

 protected:
  void computeAtts() { }
  void computeExpensiveAtts();
};

LocalEntry::LocalEntry(std::string const& name, std::string const& path,
                       struct stat& stat) :
    Entry(name, path)
{
  if (S_ISREG(stat.st_mode)) {
    atts_["kind"] = "file";
    atts_["uid"] = stringify(stat.st_uid);
    atts_["gid"] = stringify(stat.st_gid);
    atts_["mtime"] = stringify(stat.st_mtime);
    atts_["ctime"] = stringify(stat.st_ctime);
    atts_["ino"] = stringify(stat.st_ino);
    atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISLNK(stat.st_mode)) {
    atts_["kind"] = "lnk";
  } else if (S_ISSOCK(stat.st_mode)) {
    atts_["kind"] = "sock";
    atts_["uid"] = stringify(stat.st_uid);
    atts_["gid"] = stringify(stat.st_gid);
    atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISFIFO(stat.st_mode)) {
    atts_["kind"] = "fifo";
    atts_["uid"] = stringify(stat.st_uid);
    atts_["gid"] = stringify(stat.st_gid);
    atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
  } else if (S_ISBLK(stat.st_mode)) {
    atts_["kind"] = "blk";
    atts_["uid"] = stringify(stat.st_uid);
    atts_["gid"] = stringify(stat.st_gid);
    atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
    atts_["devmaj"] = stringify(major(stat.st_rdev));
    atts_["devmin"] = stringify(minor(stat.st_rdev));
  } else if (S_ISCHR(stat.st_mode)) {
    atts_["kind"] = "chr";
    atts_["uid"] = stringify(stat.st_uid);
    atts_["gid"] = stringify(stat.st_gid);
    atts_["perm"] = stringify(stat.st_mode & ~S_IFMT);
    atts_["devmaj"] = stringify(major(stat.st_rdev));
    atts_["devmin"] = stringify(minor(stat.st_rdev));
  } else {
    throw std::exception();
  }
}

// The C api is very weird.
static std::string getLink(std::string const& path, int length)
{
  char buf[length];
  ssize_t len = readlink(path.c_str(), buf, length);
  if (len < 0)
    throw std::exception();
  else if (len < length)
    return std::string(buf, len);
  else
    return getLink(path, 2*length);
}

void LocalEntry::computeExpensiveAtts()
{
  if (atts_["kind"] == "file") {
    Hash h;
    h.ofFile(path_);
    atts_["md5"] = h;
  } else if (atts_["kind"] == "lnk") {
    std::string target = getLink(path_, 128);
    atts_["targ"] = target;
  }
}

struct NameIno {
 public:
  NameIno(std::string& n, ino_t i) : name(n), ino(i) { }
  bool operator<(const NameIno& other) const {
    return ino < other.ino;
  }

  // Read all of the name/inode pairs from a directory, sorted by inode number.
  static void getNames(const std::string& path, std::vector<NameIno>& result);

  std::string name;
  ino_t ino;

};

template <class E>
static bool nameLess(E const& a, E const& b)
{
  return a->getName() < b->getName();
}

LocalDirEntryProxy LocalDirEntry::readDir(std::string const& path)
{
  struct stat rootStat;
  int result = lstat(path.c_str(), &rootStat);
  if (result != 0)
    throw std::exception();

  return LocalDirEntryProxy(new LocalDirEntry("__root__", path, rootStat));
}

void LocalDirEntry::scanDirectory()
{
  if (computed_)
    return;

  // std::cout << "Scanning: " << path_ << '\n';
  std::vector<NameIno> entries;
  NameIno::getNames(path_, entries);

  // Iterate through the entries, adding them appropriately as a file or dir.
  typedef std::vector<NameIno>::const_iterator iter;
  for (iter i = entries.begin(); i != entries.end(); ++i) {
    struct stat stat;
    std::string fullName = path_ + '/' + i->name;
    int result = lstat(fullName.c_str(), &stat);
    if (result != 0) {
      // TODO: Warn
      continue;
    }
    if (S_ISDIR(stat.st_mode)) {
      // std::cout << "d " << fullName << '\n';
      LocalDirEntryProxy node(new LocalDirEntry(i->name, fullName, stat));
      dirs_.push_back(node);
    } else {
      // std::cout << "- " << fullName << '\n';
      EntryProxy node(new LocalEntry(i->name, fullName, stat));
      files_.push_back(node);
    }
  }

  std::sort(files_.begin(), files_.end(), nameLess<EntryProxy>);
  std::sort(dirs_.begin(), dirs_.end(), nameLess<LocalDirEntryProxy>);

  computed_ = true;
}

class DirCloser {
 public:
  DirCloser(DIR* dir) : dir_(dir) { }
  ~DirCloser() {
    int r = closedir(dir_);
    // TODO: Log error if there is a problem.
  }

 private:
  DIR* dir_;

  DirCloser(DirCloser const& other);
  void operator=(DirCloser const& other);
};

void NameIno::getNames(const std::string& path, std::vector<NameIno>& result)
{
  DIR* dirp = opendir(path.c_str());
  if (dirp == NULL) {
    throw std::exception(); // TODO: more information.
  }
  DirCloser cleanup(dirp);

  while (true) {
    errno = 0;
    struct dirent* ent = readdir(dirp);
    if (ent == 0 && errno == 0)
      break;
    if (ent == 0)
      throw std::exception();
    std::string name(ent->d_name);

    if (name == "." || name == "..")
      continue;

    // Skip over integrity files.
    if (name.find("0sure.") == 0)
      continue;
    if (name.find("2sure.") == 0)
      continue;

    result.push_back(NameIno(name, ent->d_ino));
  }

  std::sort(result.begin(), result.end());
}

}
}
