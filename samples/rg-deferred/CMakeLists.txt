add_executable(RenderGraphDeferred
    deferred.cpp
)
target_link_libraries(RenderGraphDeferred PRIVATE ${SDL2_LIBRARIES})
target_include_directories(RenderGraphDeferred PRIVATE ${SDL2_INCLUDES})
target_link_libraries(RenderGraphDeferred PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET RenderGraphDeferredShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/Resources/shaders/rg-deferred
    SPIRV_DXC  ${SAKURA_BIN_DIR}/Resources/shaders/rg-deferred
)
add_dependencies(RenderGraphDeferred RenderGraphDeferredShaders)