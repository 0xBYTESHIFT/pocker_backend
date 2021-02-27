
include_directories(${PROJECT_ROOT_PATH}/include
    ${PROJECT_ROOT_PATH}/components/client
) 
if(USE_TRACY)
    include_directories(${TRACY_PATH})
endif()

add_executable(register_test main_cli.cpp)
target_link_libraries(register_test  ${CONAN_LIBS} client api)
target_compile_definitions(register_test PRIVATE TRACY_ENABLE=0)