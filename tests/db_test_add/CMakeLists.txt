
include_directories(${PROJECT_ROOT_PATH}/include
    ${PROJECT_ROOT_PATH}/components/client
) 
if(USE_TRACY)
    include_directories(${TRACY_PATH})
endif()

add_executable(db_add_test main.cpp)
target_link_libraries(db_add_test ${CONAN_LIBS})
target_compile_definitions(db_add_test PRIVATE TRACY_ENABLE=0)

add_test(db_add_test ${CMAKE_BINARY_DIR}/bin/db_add_test --db_config ${PROJECT_ROOT_PATH}/db/test_db.json)