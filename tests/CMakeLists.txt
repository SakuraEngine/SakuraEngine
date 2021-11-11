include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)
file(GLOB_RECURSE SAMPLE_SHARED_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.h ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.c ${CMAKE_CURRENT_SOURCE_DIR}/shared/*.cpp
)

add_subdirectory(memory)
add_subdirectory(render-setup)
add_subdirectory(window-exts)
