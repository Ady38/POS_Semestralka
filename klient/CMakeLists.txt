add_executable(klient klient.c menu.c)
set_target_properties(klient PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(klient PRIVATE "-std=c11" "-Wall" "-Wextra" "-pedantic")