link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(CGpu3D
    cgpu-renderer.cpp
    render-resources.cpp
    render-scene.cpp
    cgpu-3d.cpp
    thirdparty/lodepng.cpp
)
target_link_libraries(CGpu3D PUBLIC SakuraRuntime)

file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/scene.gltf 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/scene.bin
    DESTINATION ${SAKURA_BIN_DIR}/Resources
    FOLLOW_SYMLINK_CHAIN)

file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert5SG_baseColor.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert6SG_baseColor.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert7SG_baseColor.png
    DESTINATION ${SAKURA_BIN_DIR}/Resources/textures
    FOLLOW_SYMLINK_CHAIN)