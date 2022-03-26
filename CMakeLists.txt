cmake_minimum_required(VERSION 3.10)
project(skiplist_chenfei)

set(CMAKE_CXX_STANDARD 14)
set(GOOGLETEST_VERSION 1.11.0)

add_subdirectory(thirdparty/googletest)
include_directories(thirdparty/googletest1/include)
link_directories(thirdparty/googletest)
set(LIBRARIES gtest pthread)

add_executable(performance_test demo/performance.cpp)
target_link_libraries(performance_test ${LIBRARIES})

add_executable(kv_service demo/kv_service.cpp)
target_link_libraries(kv_service ${LIBRARIES})

set(CMAKE_CXX_FLAGS -w)
add_executable(unit_test utest/Skiplist_utest.cpp)
target_link_libraries(unit_test ${LIBRARIES})


