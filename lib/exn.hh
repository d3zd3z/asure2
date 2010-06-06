// Exceptions used in asure.

#ifndef __EXN_H__
#define __EXN_H__

#include <string>
#include <boost/exception/all.hpp>
#include <exception>

namespace asure {

struct exception_base: virtual std::exception, virtual boost::exception { };
struct io_error: virtual exception_base { };
struct parse_error: virtual exception_base { };

typedef boost::error_info<struct tag_errno, int> errno_code;
typedef boost::error_info<struct tag_path, std::string> path_code;
typedef boost::error_info<struct tag_message, std::string> message_code;

}

#endif
