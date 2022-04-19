cmake_minimum_required (VERSION 3.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_TEST "build with tests" ON)
option(BUILD_SPIRV_SHADER "build spirv shaders" ON)
option(BUILD_SAMPLES "build with samples" ON)
option(RUNTIME_SHARED "build as shared library" OFF)

set(SAKURA_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SAKURA_BIN_DIR ${CMAKE_BINARY_DIR}/Binaries) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
link_directories(${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)

project(SakuraRuntime)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Extract)
include(PlatformConfigure)

# include component builds
include(mimalloc) # Fetch mimalloc SOURCES
include(BOOST) # Fetch Boost SOURCES
include(EASTL) # Fetch EASTL SOURCES
include(DirectXMath)
include(GSL)
include(vulkan)
include(GPU)
include(ShaderCompile)
include(SDL2) # Set SDL2 ENVS
include(imgui) # Set ImGui ENVS
include(Tracy) # Set Tracy ENVS
# WASM
extract_file2(${CMAKE_CURRENT_SOURCE_DIR}/SDKs/wasm-clang-win.zip 
    ${SAKURA_BIN_DIR}/Debug
    ${SAKURA_BIN_DIR}/Release
)
message(STATUS ${DXMATH_INCLUDES_DIR})
include(WASM3) # Set WASM3 ENVS

if(RUNTIME_SHARED)
    add_library(SakuraRuntime SHARED)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_SHARED)
    # mimalloc
    target_compile_definitions(SakuraRuntime PUBLIC -DMI_SHARED_LIB)
    target_compile_definitions(SakuraRuntime PRIVATE -DMI_SHARED_LIB_EXPORT)
    # DLL
    target_compile_definitions(SakuraRuntime PRIVATE -DRUNTIME_API=RUNTIME_EXPORT)
    # EASTL
    target_compile_definitions(SakuraRuntime PRIVATE -DEASTL_API=EA_EXPORT)
else()
    add_library(SakuraRuntime STATIC)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_STATIC)
endif(RUNTIME_SHARED)

# apple platform frameworks
if(APPLE)
    find_library(FoundationLib CoreFoundation)
    find_library(CocoaLib Cocoa)
    find_library(MetalLib Metal)
    find_library(IOKitLib IOKit)
    set(PLATFORM_FRAMEWORKS ${FoundationLib} ${CocoaLib} ${MetalLib} ${IOKitLib})
elseif(WIN32)
    # libuv uses these frameworks
    set(PLATFORM_FRAMEWORKS psapi user32 advapi32 iphlpapi userenv ws2_32)
endif()

# MSVC natvis
if(MSVC)
    set(NATVIS_SOURCES 
        SDKs/natvis/EASTL.natvis
        SDKs/natvis/D3D12MemAlloc.natvis
    )
    message(STATUS "Add Natvis to Project: ${NATVIS_SOURCES}")
endif()

# glob sources
set(RUNTIME_SOURCES
    src/utils/build.0.c
    src/utils/build.0.cpp
    src/platform/build.0.cpp
    src/platform/build.0.c
    src/gpu/cgpu/build.0.cpp
    src/gpu/cgpu/build.1.cpp
    src/gpu/cgpu/build.2.cpp
    src/gpu/cgpu/build.0.c
    src/gpu/cgpu/build.1.c
    src/gpu/render_graph/build.0.cpp
    src/wasm/build.0.c
    src/wasm/build.0.cpp
    ${NATVIS_SOURCES}
)
if(APPLE)
    set(RUNTIME_SOURCES
        ${RUNTIME_SOURCES}
        src/platform/build.0.mm
        src/gpu/cgpu/build.0.m
        src/gpu/cgpu/build.0.mm
    )
endif()

# sources
target_sources(SakuraRuntime
    PRIVATE
    ${RUNTIME_SOURCES}
    ${EASTL_SOURCES}
    ${MIMALLOC_SOURCES}
)

# include dirs
target_include_directories(SakuraRuntime
    PUBLIC
    ${MIMALLOC_INCLUDES_DIR}
    ${BOOST_INCLUDES_DIR}
    ${EASTL_INCLUDES_DIR}
    ${DXMATH_INCLUDES_DIR}
    ${GSL_INCLUDES_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${WASM3_INCLUDES}
    ${WASM_EDGE_INCLUDES}
    ${SDL2_INCLUDES}
    ${TRACY_INCLUDES}
)

# link targets
target_link_libraries(SakuraRuntime 
    PUBLIC
    ${PLATFORM_FRAMEWORKS}
    ${WASM3_LIBRARIES}
    ${WASM_EDGE_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${TRACY_LIBRARIES}
    IMGUI
)

if(RUNTIME_SHARED)
    add_definitions(-DRUNTIME_DLL)
endif()
if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
if(BUILD_TEST)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()