link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(CGpuTexturedBox
    texture.c
)
target_link_libraries(CGpuTexturedBox PUBLIC SakuraRuntime)