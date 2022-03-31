if(TARGET_WIN)

add_executable(CGpuTriangle
    triangle.c
    hot_wasm.cpp
)
target_link_libraries(CGpuTriangle PRIVATE ${SDL2_LIBRARIES})
target_include_directories(CGpuTriangle PRIVATE ${SDL2_INCLUDES})
target_link_libraries(CGpuTriangle PUBLIC SakuraRuntime)

# Build a wasm target
#add_library(CGpuTriangleModule triangle_module.c)
#target_include_directories(CGpuTriangleModule PRIVATE ${SAKURA_ROOT_DIR}/include)

sakura_compile_shaders(TARGET CGpuTriangleShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/Resources/shaders/hot-triangle
    SPIRV_DXC  ${SAKURA_BIN_DIR}/Resources/shaders/hot-triangle
)
add_dependencies(CGpuTriangle CGpuTriangleShaders)
endif()