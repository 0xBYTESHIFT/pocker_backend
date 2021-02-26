
add_library(api api.cpp)
target_include_directories(api PRIVATE ${PROJECT_ROOT_PATH}/include)
target_link_libraries(api ${CONAN_LIBS})