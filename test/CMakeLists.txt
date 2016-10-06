set(common_files test.c
                 common.c
                 beer_assoc.c)

project(bee-test-tcp)
add_executable(bee-test-tcp
    bee_tcp.c
    ${common_files})
set_target_properties(bee-test-tcp PROPERTIES OUTPUT_NAME "bee-tcp")
target_link_libraries(bee-test-tcp beer)

project(bee-test-unix)
add_executable(bee-test-unix
    bee_unix.c
    ${common_files})
set_target_properties(bee-test-unix PROPERTIES OUTPUT_NAME "bee-unix")
target_link_libraries(bee-test-unix beer)

project(bee-test-disconnect)
add_executable(bee-test-disconnect
    bee_disconnect.c
    ${common_files})
set_target_properties(bee-test-disconnect PROPERTIES OUTPUT_NAME "bee-disconnect")
target_link_libraries(bee-test-disconnect beer)

project(bee-test-call)
add_executable(bee-test-call
    bee_call.c
    ${common_files})
set_target_properties(bee-test-call PROPERTIES OUTPUT_NAME "bee-call")
target_link_libraries(bee-test-call beer)

add_custom_target(test
    COMMAND ${PROJECT_SOURCE_DIR}/test-run.py --builddir=${PROJECT_BINARY_DIR}
            --vardir=${PROJECT_BINARY_DIR}/test/var)
