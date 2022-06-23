add_executable(Mandelbrot
    mandelbrot.c
    lodepng.c
)
target_link_libraries(Mandelbrot PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET MandelbrotShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mandelbrot.cs_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/resources/shaders/cgpu-mandelbrot
    SPIRV_DXC  ${SAKURA_BIN_DIR}/resources/shaders/cgpu-mandelbrot
)
add_dependencies(Mandelbrot MandelbrotShaders)