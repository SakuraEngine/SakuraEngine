cmake_minimum_required (VERSION 3.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(RUNTIME_SHARED TRUE)

set(SAKURA_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SAKURA_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build/Binaries) 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)
link_directories(${SAKURA_BIN_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>$<$<CONFIG:MinSizeRel>:Release>)

project(SakuraRuntime)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(Extract)
include(PlatformConfigure)
# Build
include(mimalloc) # Fetch mimalloc SOURCES
include(EASTL) # Fetch EASTL SOURCES
include(SDL2) # Fetch EASTL SOURCES
include(GPU)
include(vulkan)

if(RUNTIME_SHARED)
    add_library(SakuraRuntime SHARED)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_SHARED)
    # DLL
    target_compile_definitions(SakuraRuntime PRIVATE -DRUNTIME_API=RUNTIME_EXPORT)
    # EASTL
    target_compile_definitions(SakuraRuntime PRIVATE -DEASTL_API=EA_EXPORT)
else()
    add_library(SakuraRuntime STATIC)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_STATIC)
endif(RUNTIME_SHARED)

if(MSVC)
    set(NATVIS_SOURCES 
        SDKs/natvis/EASTL.natvis
        SDKs/natvis/D3D12MemAlloc.natvis
    )
    message(STATUS "Add Natvis to Project: ${NATVIS_SOURCES}")
endif()

set(RUNTIME_SOURCES
    src/platform/build.0.cpp
    src/gpu/cgpu/build.0.cpp
    src/gpu/cgpu/build.1.cpp
    src/gpu/cgpu/build.2.cpp
    src/gpu/cgpu/build.0.c
    src/gpu/cgpu/build.1.c
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

target_sources(SakuraRuntime
    PRIVATE
    ${RUNTIME_SOURCES}
    ${EASTL_SOURCES}
    ${MIMALLOC_SOURCES}
)
target_include_directories(SakuraRuntime
    PUBLIC
    ${MIMALLOC_INCLUDES_DIR}
    ${EASTL_INCLUDES_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(APPLE)
    find_library(FoundationLib CoreFoundation)
    find_library(CocoaLib Cocoa)
    find_library(MetalLib Metal)
    target_link_libraries(SakuraRuntime 
        PUBLIC
        ${FoundationLib}
        ${CocoaLib}
        ${MetalLib}
    )
endif()

set(BUILD_TEST TRUE)
set(USE_TEST_FRAMEWORK TRUE)
if(BUILD_TEST)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()