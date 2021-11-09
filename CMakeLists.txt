cmake_minimum_required (VERSION 3.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

project(SakuraRuntime)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(PlatformConfigure)
# Build
include(EASTL) # Fetch EASTL SOURCES
include(mimalloc) # Fetch mimalloc SOURCES

add_library(SakuraRuntime SHARED)
target_sources(SakuraRuntime
    PRIVATE
    ${EASTL_SOURCES}
    ${MIMALLOC_SOURCES}
)
target_include_directories(SakuraRuntime
    PUBLIC
    ${MIMALLOC_INCLUDES_DIR}
    ${EASTL_INCLUDES_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(SakuraRuntime PUBLIC -DEA_DLL)
add_subdirectory(tests)