cmake_minimum_required(VERSION 3.16)
project(rcd C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(rcd
    src/main.c
    src/rcd_types.c
    src/rcd_csv.c
    src/rcd_reliability.c
    src/rcd_ga.c
    src/rcd_output.c
)

target_include_directories(rcd PRIVATE include)
target_compile_options(rcd PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(rcd PRIVATE m)
