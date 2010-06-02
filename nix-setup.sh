#! /bin/sh

# On Nix, boost is not managed by pkgconfig, so cmake is unable to
# find it.  Query where the package is installed and use that.  This
# makes the assumption that the package is installed as part of the
# user profile, and not by the system, or even part of a collection.
# This is not really ideal.

# Another option would be to try and track down the symlink for one of
# the include files or libraries, and find the store base.

BOOST=`nix-env -q --out-path --no-name boost`

mkdir -p build
cd build
cmake \
	-DBOOST_ROOT=${BOOST} \
	-DCMAKE_BUILD_TYPE=DEBUG \
	..
