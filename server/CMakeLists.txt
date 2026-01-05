add_executable(server server.c)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(server PRIVATE "-std=c11" "-Wall" "-Wextra" "-pedantic")