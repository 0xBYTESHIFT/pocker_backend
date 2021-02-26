
add_library(client client.cpp)
target_include_directories(client PRIVATE ${PROJECT_ROOT_PATH}/include)
target_link_libraries(client ${CONAN_LIBS})