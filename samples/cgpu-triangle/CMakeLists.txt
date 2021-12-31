add_executable(CGpuTriangle
    triangle.c
    wasm_modules.cpp
)
target_link_libraries(CGpuTriangle PRIVATE ${SDL2_LIBRARIES})
target_include_directories(CGpuTriangle PRIVATE ${SDL2_INCLUDES})
target_link_libraries(CGpuTriangle PUBLIC SakuraRuntime)

# Build a wasm target
#add_library(CGpuTriangleModule triangle_module.c)
#target_include_directories(CGpuTriangleModule PRIVATE ${SAKURA_ROOT_DIR}/include)