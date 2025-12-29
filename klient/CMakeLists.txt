add_executable(klient)
target_sources( klient PRIVATE
        klient.c
        menu.c
)
target_compile_options(klient PRIVATE "-std=c11" "-Wall" "-Wextra" "-pedantic")