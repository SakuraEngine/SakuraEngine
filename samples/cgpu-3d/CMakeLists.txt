link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(3D
    cgpu-renderer.cpp
    render-resources.cpp
    render-scene.cpp
    cgpu-3d.cpp
    thirdparty/lodepng.cpp
)
target_link_libraries(3D PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET 3DShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/resources/shaders/cgpu-3d
    SPIRV_DXC  ${SAKURA_BIN_DIR}/resources/shaders/cgpu-3d
)
add_dependencies(3D 3DShaders)

file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/scene.gltf 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/scene.bin
    DESTINATION ${SAKURA_BIN_DIR}/resources
    FOLLOW_SYMLINK_CHAIN)

file(COPY 
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert5SG_baseColor.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert6SG_baseColor.png
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/lambert7SG_baseColor.png
    DESTINATION ${SAKURA_BIN_DIR}/resources/textures
    FOLLOW_SYMLINK_CHAIN)