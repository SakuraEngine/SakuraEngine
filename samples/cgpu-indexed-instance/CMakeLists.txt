link_libraries(${SDL2_LIBRARIES})
include_directories(${SDL2_INCLUDES})

add_executable(IndexedInstance
    indexed-instance.c
)
target_link_libraries(IndexedInstance PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET IndexedInstanceShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/resources/shaders/cgpu-indexed-instance
    SPIRV_DXC  ${SAKURA_BIN_DIR}/resources/shaders/cgpu-indexed-instance
)
add_dependencies(IndexedInstance IndexedInstanceShaders)