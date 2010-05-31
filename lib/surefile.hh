// Writing surefiles.

#ifndef __SUREFILE_H__
#define __SUREFILE_H__

#include <string>
#include "dirstream.hh"

namespace asure {

void saveSurefile(std::string baseName, stream::DirEntryProxy root);

}

#endif
