cmake_minimum_required(VERSION 3.13.0)
project(poker VERSION 0.0.1 LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1 -g -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s -ffast-math -march=native -mtune=native")
option(USE_TRACY "profiling" OFF)

if (USE_TRACY)
        find_package(Git QUIET)
        message(STATUS "tracy clone")
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tracy/)
            execute_process(COMMAND ${GIT_EXECUTABLE} clone https://github.com/wolfpld/tracy
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_SUBMOD_RESULT)
            if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git clone failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
            endif()
        endif()
        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/tracy/)
endif()

find_program(TIDY "clang-tidy-11")
# set(CMAKE_CXX_CLANG_TIDY "${TIDY};-checks=-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},bugprone-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},cppcoreguidelines-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},clang-analyzer-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},misc-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},modernize-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},performance-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},portability-*")
# set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY},readability-*")

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_CURRENT_LIST_DIR} components)
add_subdirectory(client) 

set(POKER_SRC
    src/net/tcp_connection.cpp
    src/net/tcp_server.cpp
    src/net/mes_handler.cpp
    src/api.cpp
    src/db/database_worker.cpp
    src/server.cpp
    src/user.cpp
)

if(USE_TRACY)
        set(POKER_SRC ${POKER_SRC}
                ${CMAKE_CURRENT_SOURCE_DIR}/tracy/TracyClient.cpp
                )
endif()

add_executable(poker main.cpp ${POKER_SRC})
target_include_directories(poker PRIVATE include)
target_link_libraries(poker ${CONAN_LIBS})
target_compile_options(poker PRIVATE -Wall -Wextra)

if(USE_TRACY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE=1)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)