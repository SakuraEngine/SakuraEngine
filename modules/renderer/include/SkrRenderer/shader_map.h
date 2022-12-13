#pragma once
#include "platform/configure.h"
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include "cgpu/api.h"

typedef struct skr_shader_map_key_t skr_shader_map_key_t;
typedef struct skr_shader_map_t skr_shader_map_t;
typedef const struct skr_shader_map_t* skr_shader_map_id;
typedef struct SkrShaderMapDescriptor SkrShaderMapDescriptor;

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
skr_shader_map_id skr_shader_map_create(const struct SkrShaderMapDescriptor* desc);

// (RC) request install for specific platform shader identifier
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
ESkrShaderMapShaderStatus skr_shader_map_request_install(skr_shader_map_id shaderMap, const skr_platform_shader_hash_t* key);

// fetch shader in shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
CGPUShaderLibraryId skr_shader_map_find_shader(skr_shader_map_id shaderMap, const skr_platform_shader_hash_t& id);

// (RC) free shader to shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_shader_map_free_shader(skr_shader_map_id shaderMap, const skr_platform_shader_hash_t& id);

// shader map new frame
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_shader_map_new_frame(skr_shader_map_id shaderMap, uint64_t frame_index);

// do (RC) garbage collect once for shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_shader_map_garbage_collect(skr_shader_map_id shaderMap, uint64_t critical_frame);

// request uninstall for specific platform shader identifier
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
bool skr_shader_map_request_uninstall(skr_shader_map_id shaderMap, const skr_platform_shader_hash_t* key);

// free a shader map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_shader_map_free(skr_shader_map_id shader_map);

#ifdef __cplusplus
// {guid} -> shader collection 
//              -> {stable_hash} -> multi_shader 
//                                      -> {stable_hash} -> shader_identifier (final)
struct SKR_RENDERER_API skr_shader_map_t
{
    virtual CGPUShaderLibraryId find_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT = 0;
    virtual void free_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT = 0;

    virtual ESkrShaderMapShaderStatus install_shader(const skr_platform_shader_hash_t& id, skr_shader_collection_resource_t* collection) SKR_NOEXCEPT = 0;
    virtual bool uninstall_shader(const skr_platform_shader_hash_t& id) SKR_NOEXCEPT = 0;

    virtual void new_frame(uint64_t frame_index) SKR_NOEXCEPT = 0;
    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT = 0;

    static skr_shader_map_id Create(const struct SkrShaderMapDescriptor* desc) SKR_NOEXCEPT;
    static bool Free(skr_shader_map_id shader_map) SKR_NOEXCEPT;
};
#endif