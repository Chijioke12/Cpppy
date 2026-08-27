cmake_minimum_required(VERSION 3.15)
project(PhysicsGameCpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cpp/include)

set(SOURCES
    cpp/src/main.cpp
)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_executable(index ${SOURCES})

    set_target_properties(index PROPERTIES
        LINK_FLAGS "-O3 -s USE_SDL=2 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell_minimal.html"
    )
else()
    # Native build with system SDL2
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})

    add_executable(physics_game ${SOURCES})
    target_link_libraries(physics_game PRIVATE ${SDL2_LIBRARIES})
endif()
