/* File integrity checking.
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include <getopt.h>

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

std::string command;
string sureFile = "2sure";

void parseArgs(int argc, char const* const* argv)
{
  static struct option long_options[] = {
    {"surefile", 1, 0, 'f'},
    {"file", 1, 0, 'f'},
    {"help", 0, 0, '?'},
    {0, 0, 0, 0}
  };

  int option_index = 0;
  while (true) {
    int c = getopt_long(argc, const_cast<char* const*>(argv), "?f:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      case 0:
        if (!command.empty())
          throw usage_error("expecting a single command");
        command = optarg;
        break;

      case 'f':
        sureFile = optarg;
        break;

      case '?':
        throw usage_error("");

      default:
        throw usage_error("invalid usage");
    }
  }

  if (optind == argc)
    throw usage_error("expecting a command");
  if (optind != argc - 1)
    throw usage_error("expecting only a single command");
  command = argv[optind];
}

}

int main(int argc, char const* const* argv)
{
  try {
    parseArgs(argc, argv);

    if (command == "scan") {
      std::auto_ptr<NodeIterator> root(asure::tree::walkTree("."));
      asure::SurefileSaver::save(sureFile, *root);
    } else if (command == "show") {
      std::string name = sureFile;
      name += asure::extensions::base;
      std::auto_ptr<NodeIterator> root(asure::loadSurefile(name));
      show(*root);
    } else if (command == "check") {
      std::string name = sureFile;
      name += asure::extensions::base;
      std::auto_ptr<NodeIterator> surefile(asure::loadSurefile(name));
      std::auto_ptr<NodeIterator> curtree(asure::tree::walkTree("."));
      asure::compareTrees(*surefile, *curtree);
    } else if (command == "signoff") {
      std::string name1 = sureFile;
      name1 += asure::extensions::bak;
      std::string name2 = sureFile;
      name2 += asure::extensions::base;
      std::auto_ptr<NodeIterator> bakfile(asure::loadSurefile(name1));
      std::auto_ptr<NodeIterator> curfile(asure::loadSurefile(name2));
      asure::compareTrees(*bakfile, *curfile);
    } else if (command == "update") {
      std::string sureName = sureFile + asure::extensions::base;
      std::auto_ptr<NodeIterator> surefile(asure::loadSurefile(sureName));
      std::auto_ptr<NodeIterator> tree(asure::tree::walkTree("."));
      asure::SurefileSaver saver(sureFile);
      asure::updateTree(*surefile, *tree, saver);
    } else if (command == "walk") {
      std::auto_ptr<NodeIterator> root(asure::tree::walkTree("."));
      show(*root);
    } else
      throw usage_error("unknown command: " + command);
  }
  catch (usage_error& err) {
    cout << "Asure, version 2.01\n";
    cout << "Usage: asure [{-f|--surefile|--file} name] {scan|update|check|signoff|show|walk}\n\n";
    cout << err.what() << '\n';
  }
  catch (asure::Exception_base& e) {
    cout << "Uncaught exception: " << e.what() << '\n';
    std::exit(1);
  }
  catch (int ret) {
    cout << "Raised: " << ret << '\n';
  }
}
