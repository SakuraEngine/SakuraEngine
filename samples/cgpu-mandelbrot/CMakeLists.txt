add_executable(CGpuMandelbrot
    mandelbrot.c
    lodepng.c
)
target_link_libraries(CGpuMandelbrot PUBLIC SakuraRuntime)

sakura_compile_shaders(TARGET CGpuMandelbrotShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/shaders/mandelbrot.cs_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/Resources/shaders/cgpu-mandelbrot
    SPIRV_DXC  ${SAKURA_BIN_DIR}/Resources/shaders/cgpu-mandelbrot
)
add_dependencies(CGpuMandelbrot CGpuMandelbrotShaders)