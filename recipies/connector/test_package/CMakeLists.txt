project(PackageTest CXX)
cmake_minimum_required(VERSION 3.0)
find_package(Threads REQUIRED)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(connector_test connector_test.cpp)
target_link_libraries(connector_test PRIVATE 
    ${CMAKE_THREAD_LIBS_INIT}
    ${CONAN_LIBS}
    dl)

# CTest is a testing tool that can be used to test your project.
enable_testing()
add_test(NAME connector_test
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
         COMMAND example)
