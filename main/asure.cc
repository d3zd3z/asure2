/* File integrity checking.
 */

#include <iostream>
#include <string>

#include "tree-local.hh"
#include "surefile.hh"

using std::string;
using std::cout;

using asure::tree::DirEntryProxy;

class usage_error : public std::exception {
  public:
    usage_error(string const& message) throw() : message_(message) { }
    ~usage_error() throw() { }
    const char* what() const throw() { return message_.c_str(); }
  private:
    const string message_;
};

int main(int argc, char const* const* argv)
{
  try {
    if (argc != 2)
      throw usage_error("expecting a single argument");

    string const command = argv[1];
    if (command == "scan") {
      string root = ".";
      DirEntryProxy here = asure::tree::LocalDirEntry::readDir(root);
      asure::saveSurefile("2sure", here);
    } else
      throw usage_error("unknown command");
  }
  catch (usage_error& err) {
    cout << "Asure, version 2.00\n";
    cout << "Usage: asure {scan}\n\n";
    cout << err.what() << '\n';
  }
  catch (int ret) {
    cout << "Raised: " << ret << '\n';
  }
}
