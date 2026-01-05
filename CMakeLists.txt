cmake_minimum_required(VERSION 3.15)

project(
        hadik
        VERSION 0.1.0
        LANGUAGES C
)

# Set the output directory for all executables
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(klient)
add_subdirectory(server)
