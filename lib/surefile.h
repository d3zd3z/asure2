// Writing surefiles.

#ifndef __SUREFILE_H__
#define __SUREFILE_H__

#include <string>
#include "dirstream.h"

namespace asure {

void saveSurefile(std::string baseName, stream::DirEntryProxy root);

}

#endif
