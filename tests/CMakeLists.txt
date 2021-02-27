
add_subdirectory(register_test)
add_subdirectory(login_test)

include_directories(${PROJECT_ROOT_PATH}/include
    ${PROJECT_ROOT_PATH}/components/client
) 
if(USE_TRACY)
    include_directories(${TRACY_PATH})
endif()

add_executable(test_runner test_runner.cpp)
target_link_libraries(test_runner ${CONAN_LIBS})
target_compile_definitions(test_runner  PRIVATE TRACY_ENABLE=0)

add_test(register_test ${CMAKE_BINARY_DIR}/bin/test_runner
    --serv_path ${CMAKE_BINARY_DIR}/bin/poker
    --test_path ${CMAKE_BINARY_DIR}/bin/register_test
    --port 9000 --db_config ${PROJECT_ROOT_PATH}/db/config.json)
add_test(login_test ${CMAKE_BINARY_DIR}/bin/test_runner
    --serv_path ${CMAKE_BINARY_DIR}/bin/poker
    --test_path ${CMAKE_BINARY_DIR}/bin/login_test
    --port 9000 --db_config ${PROJECT_ROOT_PATH}/db/config.json)