add_executable(server)
target_sources( server PRIVATE
        server.c
)
target_compile_options(server PRIVATE "-std=c11" "-Wall" "-Wextra" "-pedantic")