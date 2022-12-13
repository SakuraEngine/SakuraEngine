#pragma once
#include "platform/configure.h"
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/shader_hash.h"
#include "cgpu/api.h"

typedef struct skr_shader_map_key_t skr_shader_map_key_t;
typedef struct skr_shader_map_t skr_shader_map_t;
typedef struct skr_shader_map_t* skr_shader_map_id;
typedef struct skr_shader_map_root_t skr_shader_map_root_t;

typedef enum ESkrShaderMapShaderStatus
{
    SKR_SHADER_MAP_SHADER_STATUS_NONE,
    SKR_SHADER_MAP_SHADER_STATUS_REQUESTED,
    SKR_SHADER_MAP_SHADER_STATUS_LOADING,
    SKR_SHADER_MAP_SHADER_STATUS_LOADED,
    SKR_SHADER_MAP_SHADER_STATUS_FAILED,
    SKR_SHADER_MAP_SHADER_STATUS_INSTALLED,
    SKR_SHADER_MAP_SHADER_STATUS_UNINSTALLED,
    SKR_SHADER_MAP_SHADER_STATUS_COUNT,
    SKR_SHADER_MAP_SHADER_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} ESkrShaderMapShaderStatus;

// create a shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
skr_shader_map_id skr_shader_map_create(const struct skr_shader_map_root_t* desc);

// thread-safe.
// (RC) request install for specific platform shader identifier
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
ESkrShaderMapShaderStatus skr_shader_map_install_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* key);

// thread-safe.
// fetch shader in shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
CGPUShaderLibraryId skr_shader_map_find_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* id);

// thread-safe.
// (RC) free shader in shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_shader_map_free_shader(skr_shader_map_id shaderMap, const skr_platform_shader_identifier_t* id);

// shader map new frame
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_shader_map_new_frame(skr_shader_map_id shaderMap, uint64_t frame_index);

// thread-safe.
// do (RC) garbage collect once for shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_shader_map_garbage_collect(skr_shader_map_id shaderMap, uint64_t critical_frame);

// free a shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_shader_map_free(skr_shader_map_id shader_map);

typedef struct skr_shader_map_root_t {
    struct skr_vfs_t* bytecode_vfs = nullptr;
    struct skr_io_ram_service_t* ram_service = nullptr;
    SRenderDeviceId render_device = nullptr;
    skr_threaded_service_t* aux_service = nullptr;
} skr_shader_map_root_t;

#ifdef __cplusplus
// {guid} -> shader collection 
//              -> {stable_hash} -> multi_shader 
//                                      -> {stable_hash} -> shader_identifier (final)
struct SKR_RENDERER_API skr_shader_map_t
{
    virtual ~skr_shader_map_t() = default;

    virtual ESkrShaderMapShaderStatus install_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT = 0;
    virtual CGPUShaderLibraryId find_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT = 0;
    virtual bool free_shader(const skr_platform_shader_identifier_t& id) SKR_NOEXCEPT = 0;

    virtual void new_frame(uint64_t frame_index) SKR_NOEXCEPT = 0;
    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT = 0;

    static skr_shader_map_id Create(const struct skr_shader_map_root_t* desc) SKR_NOEXCEPT;
    static bool Free(skr_shader_map_id shader_map) SKR_NOEXCEPT;
};
#endif