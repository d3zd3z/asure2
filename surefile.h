// Writing surefiles.

#ifndef __SUREFILE_H__
#define __SUREFILE_H__

#include <string>
#include "dirstream.h"

namespace asure {

void saveSurefile(std::string surePath, std::string tmpPath, stream::DirEntryProxy root);

}

#endif
