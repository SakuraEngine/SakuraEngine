link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(CGpuTriangle
    triangle.c
)
target_link_libraries(CGpuTriangle PUBLIC SakuraRuntime)