add_executable(RenderGraphDeferred
    deferred.cpp
)
target_link_libraries(RenderGraphDeferred PRIVATE ${SDL2_LIBRARIES})
target_include_directories(RenderGraphDeferred PRIVATE ${SDL2_INCLUDES})
target_link_libraries(RenderGraphDeferred PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET RenderGraphDeferredShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gbuffer_vs.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/gbuffer_fs.ps_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/screen_vs.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/lighting_fs.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/Resources/shaders/rg-deferred
    SPIRV_DXC  ${SAKURA_BIN_DIR}/Resources/shaders/rg-deferred
)
add_dependencies(RenderGraphDeferred RenderGraphDeferredShaders)