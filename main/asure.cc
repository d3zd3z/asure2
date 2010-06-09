/* File integrity checking.
 */

#include <iostream>
#include <string>

#include "compare.hh"
#include "tree-local.hh"
#include "surefile.hh"
#include "exn.hh"

using std::string;
using std::cout;

class usage_error : public std::exception {
  public:
    usage_error(string const& message) throw() : message_(message) { }
    ~usage_error() throw() { }
    const char* what() const throw() { return message_.c_str(); }
  private:
    const string message_;
};

namespace {
using asure::tree::Node;
using asure::tree::NodeIterator;

void indent(int depth)
{
  for (int i = 0; i < 2*depth; ++i)
    std::cout << ' ';
}

void showAtts(Node::Atts const& atts)
{
  typedef Node::Atts::const_iterator iter;
  iter const& end = atts.end();
  for (iter i = atts.begin(); i != end; ++i) {
    std::cout << ' ' << i->first << '=' << i->second;
  }
}

void show(NodeIterator& root)
{
  int depth = 0;

  while (!root.empty()) {
    Node const& here = *root;

    switch (here.getKind()) {
      case Node::ENTER :
        indent(depth);
        std::cout << "d " << here.getName();
        showAtts(here.getAtts());
        std::cout << '\n';
        ++depth;
        break;
      case Node::LEAVE :
        --depth;
        indent(depth);
        std::cout << "u\n";
        break;
      case Node::MARK :
        indent(depth);
        std::cout << "-\n";
        break;
      case Node::NODE :
        indent(depth);
        std::cout << "f " << here.getName();
        showAtts(here.getAtts());
        std::cout << '|';
        showAtts(here.getExpensiveAtts());
        std::cout << '\n';
        break;
    }

    ++root;
  }
}

}

int main(int argc, char const* const* argv)
{
  try {
    if (argc != 2)
      throw usage_error("expecting a single argument");

    string const command = argv[1];
    if (command == "scan") {
      std::auto_ptr<NodeIterator> root(asure::tree::walkTree("."));
      asure::saveSurefile("2sure", *root);
    } else if (command == "show") {
      std::string name = "2sure";
      name += asure::extensions::base;
      std::auto_ptr<NodeIterator> root(asure::loadSurefile(name));
      show(*root);
    } else if (command == "check") {
      std::string name = "2sure";
      name += asure::extensions::base;
      std::auto_ptr<NodeIterator> surefile(asure::loadSurefile(name));
      std::auto_ptr<NodeIterator> curtree(asure::tree::walkTree("."));
      asure::compareTrees(*surefile, *curtree);
    } else if (command == "signoff") {
      std::string name1 = "2sure";
      name1 += asure::extensions::bak;
      std::string name2 = "2sure";
      name2 += asure::extensions::base;
      std::auto_ptr<NodeIterator> bakfile(asure::loadSurefile(name1));
      std::auto_ptr<NodeIterator> curfile(asure::loadSurefile(name2));
      asure::compareTrees(*bakfile, *curfile);
    } else if (command == "walk") {
      std::auto_ptr<NodeIterator> root(asure::tree::walkTree("."));
      show(*root);
    } else
      throw usage_error("unknown command");
  }
  catch (usage_error& err) {
    cout << "Asure, version 2.00\n";
    cout << "Usage: asure {scan}\n\n";
    cout << err.what() << '\n';
  }
  catch (asure::exception_base& e) {
    cout << boost::diagnostic_information(e) << '\n';
    exit(1);
  }
  catch (int ret) {
    cout << "Raised: " << ret << '\n';
  }
}
