cmake_minimum_required (VERSION 3.3)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if(UNIX)
    set(PLATFORM_INCLUDES ${PLATFORM_INCLUDES} include/Platform/Unix)
    set(PLATFORM_INCLUDES ${PLATFORM_INCLUDES} include/Platform/DirectXMath)
    add_definitions(-D "SAKURA_TARGET_PLATFORM_UNIX")
    if(APPLE)
        add_definitions(-D "SAKURA_TARGET_PLATFORM_MACOS")
        set(SAKURA_PLATFORM "mac")
    elseif(${CMAKE_SYSTEM_NAME} MATCHES ANDROID)
        add_definitions(-D "SAKURA_TARGET_PLATFORM_ANDROID")
        set(SAKURA_PLATFORM "android")
        set(ANDROID 1)
    elseif(${CMAKE_SYSTEM_NAME} MATCHES Emscripten)
        add_definitions(-D "SAKURA_TARGET_PLATFORM_EMSCRIPTEN")
        message(STATUS "Platform Web")
        set(SAKURA_PLATFORM "web")
        set(WA 1)   #Web
    else(APPLE)
        add_definitions(-D "SAKURA_TARGET_PLATFORM_LINUX")
        set(SAKURA_PLATFORM "linux")
    endif(APPLE)
elseif(WIN32)
    add_definitions(-D "SAKURA_TARGET_PLATFORM_WIN")
    set(SAKURA_PLATFORM "windows")
    message(STATUS "Platform WIN")
elseif(__COMPILER_PS5)
    message(STATUS "Platform PS5")
    set(SAKURA_PLATFORM "prospero")
    add_definitions(-D "SAKURA_TARGET_PLATFORM_PLAYSTATION")
endif(UNIX)

project(SakuraRuntime)

include(EASTL) # Fetch EASTL SOURCES
include(mimalloc) # Fetch mimalloc SOURCES

add_library(SakuraRuntime)
target_sources(SakuraRuntime
    PRIVATE
    ${EASTL_SOURCES}
    ${MIMALLOC_SOURCES}
)
target_include_directories(SakuraRuntime
    PUBLIC
    ${MIMALLOC_INCLUDES_DIR}
)
