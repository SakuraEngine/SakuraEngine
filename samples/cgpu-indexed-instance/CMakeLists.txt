link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(CGpuIndexedInstance
    indexed-instance.c
)
target_link_libraries(CGpuIndexedInstance PUBLIC SakuraRuntime)