// Exceptions used in asure.

#ifndef __EXN_H__
#define __EXN_H__

#include <errno.h>
#include <cstring>
#include <string>
#include <exception>

namespace asure {

// Exceptions within Asure have this as a base.
class Exception_base : public std::exception {
 public:
  Exception_base(std::string const& kind_) throw() : kind(kind_) { }
  ~Exception_base() throw() { }

  virtual char const* what() const throw() {
    return kind.c_str();
  }

 protected:
  std::string kind;
};

// IO errors indicate some type of failure with a system call.  These will
// capture an operation, a path, and the value of errno.
class IO_error : public Exception_base {
 public:
  IO_error(std::string const& call_, std::string const& path_) throw () :
      Exception_base(std::string("IO_error: ") + call_ + " (" + path_ + "): " +
                     std::strerror(errno)) { }
  ~IO_error() throw() { }
};

// Errors when parsing the surefiles.
class Parse_error : public Exception_base {
 public:
  Parse_error(std::string const& msg_) throw () :
      Exception_base(std::string("parse error: ") + msg_) { }
};

// struct io_error: virtual exception_base { };
// struct parse_error: virtual exception_base { };
// struct compare_error : virtual exception_base { };

}

#endif
