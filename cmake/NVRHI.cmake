include(CMakeDependentOption)
set(NVRHI_COMMON_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nvrhi/include CACHE STRING INTERNAL FORCE)
set(NVRHI_COMMON_SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nvrhi/src CACHE STRING INTERNAL FORCE)
set(NVRHI_INCLUDES_DIRS ${NVRHI_INCLUDES_DIRS} ${NVRHI_COMMON_INCLUDES_DIR})

option(NVRHI_WITH_VULKAN "Build the NVRHI Vulkan backend" ON)
cmake_dependent_option(NVRHI_WITH_NVAPI "Include NVAPI support (requires NVAPI SDK)" OFF "WIN32" OFF)
cmake_dependent_option(NVRHI_WITH_DX11 "Build the NVRHI D3D11 backend" ON "WIN32" OFF)
cmake_dependent_option(NVRHI_WITH_DX12 "Build the NVRHI D3D12 backend" ON "WIN32" OFF)

if(RUNTIME_SHARED)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif(RUNTIME_SHARED)

if(NVRHI_WITH_VULKAN)
    set(VK_HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/Vulkan-Headers CACHE STRING INTERNAL FORCE)
    set(NVRHI_INCLUDES_DIRS ${NVRHI_INCLUDES_DIRS} ${VK_HEADERS_DIR})
endif(NVRHI_WITH_VULKAN)

set(include_common
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/nvrhi.h
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/utils.h
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/common/containers.h
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/common/misc.h
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/common/resource.h)
set(src_common
    ${NVRHI_COMMON_SOURCES_DIR}/common/format-info.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/common/misc.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/common/shader-blob.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/common/state-tracking.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/common/state-tracking.h
    ${NVRHI_COMMON_SOURCES_DIR}/common/utils.cpp)

if(MSVC)
    set(misc_common tools/nvrhi.natvis)
else()
    set(misc_common "")
endif()

set(include_validation
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/validation.h)
set(src_validation
    ${NVRHI_COMMON_SOURCES_DIR}/validation/validation-commandlist.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/validation/validation-device.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/validation/validation-backend.h)
set(NVRHI_SOURCES ${NVRHI_SOURCES} ${src_common} ${src_validation})
set(NVRHI_HEADERS ${NVRHI_HEADERS} ${include_common} ${include_validation})

set(include_d3d11
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/d3d11.h)
set(src_d3d11
    ${NVRHI_COMMON_SOURCES_DIR}/common/dxgi-format.h
    ${NVRHI_COMMON_SOURCES_DIR}/common/dxgi-format.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-buffer.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-commandlist.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-compute.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-constants.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-backend.h
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-device.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-graphics.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-queries.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-resource-bindings.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-shader.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d11/d3d11-texture.cpp)
if(NVRHI_WITH_DX11)
    set(NVRHI_SOURCES ${NVRHI_SOURCES} ${src_d3d11})
    set(NVRHI_HEADERS ${NVRHI_HEADERS} ${include_d3d11})
endif(NVRHI_WITH_DX11)

set(include_d3d12
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/d3d12.h)
set(src_d3d12
    ${NVRHI_COMMON_SOURCES_DIR}/common/dxgi-format.h
    ${NVRHI_COMMON_SOURCES_DIR}/common/dxgi-format.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/common/versioning.h
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-buffer.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-commandlist.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-compute.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-constants.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-backend.h
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-descriptor-heap.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-device.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-graphics.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-meshlets.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-queries.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-raytracing.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-resource-bindings.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-shader.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-state-tracking.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-texture.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/d3d12/d3d12-upload.cpp)
if(NVRHI_WITH_DX12)
    set(NVRHI_SOURCES ${NVRHI_SOURCES} ${src_d3d12})
    set(NVRHI_HEADERS ${NVRHI_HEADERS} ${include_d3d12})
endif(NVRHI_WITH_DX12)


set(include_vk
    ${NVRHI_COMMON_INCLUDES_DIR}/nvrhi/vulkan.h)
set(src_vk
    ${NVRHI_COMMON_SOURCES_DIR}/common/versioning.h
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-allocator.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-buffer.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-commandlist.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-compute.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-constants.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-device.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-graphics.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-meshlets.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-queries.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-queue.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-raytracing.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-resource-bindings.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-shader.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-staging-texture.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-state-tracking.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-texture.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-upload.cpp
    ${NVRHI_COMMON_SOURCES_DIR}/vulkan/vulkan-backend.h)
if(NVRHI_WITH_VULKAN)
    set(NVRHI_SOURCES ${NVRHI_SOURCES} ${src_vk})
    set(NVRHI_HEADERS ${NVRHI_HEADERS} ${include_vk})
endif(NVRHI_WITH_VULKAN)

set(NVRHI_SOURCES ${NVRHI_SOURCES} CACHE STRING INTERNAL FORCE)
set(NVRHI_INCLUDES_DIRS ${NVRHI_INCLUDES_DIRS} CACHE STRING INTERNAL FORCE)

message(STATUS "NVRHI")