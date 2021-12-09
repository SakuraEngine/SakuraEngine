add_executable(CGpuMandelbrot
    mandelbrot.c
    lodepng.c
)

target_link_libraries(CGpuMandelbrot PUBLIC SakuraRuntime)