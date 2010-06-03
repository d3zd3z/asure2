file(GLOB TestSrc *.cc)
add_definitions(-DBOOST_TEST_DYN_LINK)
add_executable(asure-test ${TestSrc})
include_directories(
	${UUID_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIRS})
target_link_libraries(asure-test sure ${Boost_LIBRARIES}
	${OPENSSL_LDFLAGS}
	${UUID_LDFLAGS})
