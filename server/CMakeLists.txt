add_executable(server
        server.c
        game.c
        settings.c
        world.c
        game.c
        settings.c
        snake.c
        world.c)
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_compile_options(server PRIVATE "-std=c11" "-Wall" "-Wextra" "-pedantic")