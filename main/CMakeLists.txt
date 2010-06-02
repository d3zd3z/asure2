file(GLOB MainSrc *.cc)
add_executable(asure ${MainSrc})
target_link_libraries(asure sure ${Boost_IOSTREAMS_LIBRARY}
	${OPENSSL_LIBRARIES})
