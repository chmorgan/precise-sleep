cmake_minimum_required(VERSION 3.27)

project(precise-sleep)

set(CMAKE_CXX_STANDARD 14)
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(main
    main.cpp
)

add_custom_target(plot
        COMMAND cargo build
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/plot
)

add_custom_target(plot-images
    COMMAND $<TARGET_FILE:main>
    COMMAND cd ${CMAKE_SOURCE_DIR}/plot && cargo run -- --results=${CMAKE_BINARY_DIR}/results.csv
    DEPENDS main
)
