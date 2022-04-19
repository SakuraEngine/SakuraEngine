set(IMGUI_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui CACHE STRING INTERNAL FORCE)
set(IMGUI_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui/include CACHE STRING INTERNAL FORCE)

set(IMGUI_sources
    ${IMGUI_SOURCES_DIR}/unitybuild.cpp
)
file(GLOB_RECURSE IMGUI_headers 
    ${IMGUI_INCLUDES_DIR}/*.h 
    ${IMGUI_INCLUDES_DIR}/*.hpp
)

set(IMGUI_SOURCES ${IMGUI_sources} CACHE STRING INTERNAL FORCE)
set(IMGUI_HEADERS ${IMGUI_headers} CACHE STRING INTERNAL FORCE)

add_library(IMGUI STATIC)
target_sources(IMGUI PUBLIC ${IMGUI_SOURCES})
target_include_directories(IMGUI PUBLIC ${IMGUI_INCLUDES_DIR})

sakura_compile_shaders(TARGET ImGuiShaders
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui_vertex.vs_6_0.hlsl
            ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/imgui_fragment.ps_6_0.hlsl
    DXIL    ${SAKURA_BIN_DIR}/Resources/shaders/
    SPIRV_DXC  ${SAKURA_BIN_DIR}/Resources/shaders/
)
add_dependencies(IMGUI ImGuiShaders)

file(INSTALL ${CMAKE_CURRENT_SOURCE_DIR}/SDKs/SourceSansPro-Regular.ttf
    DESTINATION ${SAKURA_BIN_DIR}/Resources/font)