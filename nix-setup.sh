#! /bin/sh

# Nix doesn't seem to provide a nice way of finding things in the user
# environment, so search ourselves.

search_path=(
	~/.nix-profile
	/var/run/current-system/sw
	)

reduce()
{
	#echo "Reduce $1"
	if [ -L "$1" ]; then
		nix-store -q -R "$1" | tail -1
		return
	elif [ -f "$1" ]; then
		reduce `dirname "$1"`
		return
	else
		echo "Unknown entity $1"
		exit 1
	fi
}

# Scan and try to find a store path, for the symlink named $1.
find_store()
{
	for p in "${search_path[@]}"; do
		d="${p}/$1"
		#echo "try $d"
		if [ -a "$d" ]; then
			reduce "$d"
			return
		fi
	done
	echo "Unable to find path for $1"
	exit 1
}

BOOST=`find_store include/boost/version.hpp`

# Find libpthread (and librt).
libc=`find_store lib/libc.so`
bzip=`find_store lib/libbz2.so`

mkdir -p build
cd build
export CMAKE_PREFIX_PATH="${libc}:${bzip}"
cmake \
	-DBOOST_ROOT=${BOOST} \
	-DCMAKE_BUILD_TYPE=DEBUG \
	..
