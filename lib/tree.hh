// Directory trees.

#ifndef __TREE_H__
#define __TREE_H__

#include <boost/noncopyable.hpp>
#include <list>
#include <map>
#include <string>
#include <tr1/memory>

namespace asure {
namespace tree {

// Directory iteration is a depth-first traversal of a filesystem tree.
// Each directory is recursively defined as:
//  subdir = ENTER {subdir} MARK {NODE} LEAVE
// MARK is a marker that separates the subdirectories from the files within a
// directory.  The {} notation indicates zero or more instances of that item.
//
// These nodes are visited linearly.  ENTER, and NODE have a name and
// attributes, and the others do not.

class Node : boost::noncopyable {
 public:
  enum Kind {
    ENTER, MARK, NODE, LEAVE
  };
  typedef std::map<std::string, std::string> Atts;

  virtual ~Node() = 0;

  virtual Kind getKind() const = 0;
  virtual std::string const& getName() const = 0;
  virtual Atts const& getAtts() const = 0;

  // Note that the expensive atts are computed fresh, and a new set of atts is
  // returned.
  virtual Atts getExpensiveAtts() const { return Atts(); }

 protected:
  // Utility names:
  static std::string const emptyName;
  static Atts const emptyAtts;
};

// A tree visitor visits each node in the above order.  These aren't quite
// regular iterators, since the results are by reference, and the ending is
// determined with the empty() query.  The ++ operator also returns void, since
// this iterator not copied.

class NodeIterator : boost::noncopyable {
 public:
  virtual ~NodeIterator() = 0;

  virtual bool empty() const = 0;
  virtual void operator++() = 0;
  virtual Node const& operator*() const = 0;
};

}
}

#endif
