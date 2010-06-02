file(GLOB TestSrc *.cc)
add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(asure-test ${TestSrc})
target_link_libraries(asure-test sure ${Boost_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${UUID_LIBRARIES})
