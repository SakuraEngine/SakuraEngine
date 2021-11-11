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
include(NVRHI) # Fetch EASTL SOURCES

if(RUNTIME_SHARED)
    add_library(SakuraRuntime SHARED)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_SHARED)
    # DLL
    target_compile_definitions(SakuraRuntime PRIVATE -DRUNTIME_API=RUNTIME_EXPORT)
    # EASTL
    target_compile_definitions(SakuraRuntime PRIVATE -DEASTL_API=EA_EXPORT)
    # NVRHI
    target_compile_definitions(SakuraRuntime PRIVATE NVRHI_SHARED_LIBRARY_BUILD=1)
    target_compile_definitions(SakuraRuntime PRIVATE VULKAN_HPP_STORAGE_SHARED VULKAN_HPP_STORAGE_SHARED_EXPORT)
else()
    add_library(SakuraRuntime STATIC)
    target_compile_definitions(SakuraRuntime PUBLIC -DRUNTIME_STATIC)
endif(RUNTIME_SHARED)

set(RUNTIME_SOURCES
    src/platform/unitybuild.cpp
)

target_sources(SakuraRuntime
    PRIVATE
    ${RUNTIME_SOURCES}
    ${EASTL_SOURCES}
    ${MIMALLOC_SOURCES}
    ${NVRHI_SOURCES}
)
target_include_directories(SakuraRuntime
    PUBLIC
    ${MIMALLOC_INCLUDES_DIR}
    ${EASTL_INCLUDES_DIR}
    ${NVRHI_INCLUDES_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_subdirectory(tests)