link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(CGpu3D
    cgpu-renderer.cpp
)
target_link_libraries(CGpu3D PUBLIC SakuraRuntime)