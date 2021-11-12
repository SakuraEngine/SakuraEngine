link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(RenderSetup setup.cpp ${SAMPLE_SHARED_SOURCE})
target_link_libraries(RenderSetup SakuraRuntime)