if(TARGET_WIN)

add_executable(Triangle
    triangle.c
    hot_wasm.cpp
)
target_link_libraries(Triangle PRIVATE ${SDL2_LIBRARIES})
target_include_directories(Triangle PRIVATE ${SDL2_INCLUDES})
target_link_libraries(Triangle PUBLIC SakuraRuntime)

# Build a wasm target
#add_library(TriangleModule triangle_module.c)
#target_include_directories(TriangleModule PRIVATE ${SAKURA_ROOT_DIR}/include)

sakura_compile_shaders(TARGET TriangleShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vertex_shader.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/fragment_shader.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/resources/shaders/hot-triangle
    SPIRV_DXC  ${SAKURA_BIN_DIR}/resources/shaders/hot-triangle
)
add_dependencies(Triangle TriangleShaders)
endif()