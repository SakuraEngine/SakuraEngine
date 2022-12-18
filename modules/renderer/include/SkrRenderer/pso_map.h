#pragma once
#include "platform/configure.h"
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include "cgpu/api.h"

typedef struct skr_pso_map_key_t skr_pso_map_key_t;
typedef struct skr_pso_map_key_t* skr_pso_map_key_id;
typedef struct skr_pso_map_t* skr_pso_map_id;
typedef struct skr_pso_map_root_t skr_pso_map_root_t;

typedef enum ESkrPSOMapPSOStatus
{
    SKR_PSO_MAP_PSO_STATUS_NONE,
    SKR_PSO_MAP_PSO_STATUS_REQUESTED,
    SKR_PSO_MAP_PSO_STATUS_FAILED,
    SKR_PSO_MAP_PSO_STATUS_INSTALLED,
    SKR_PSO_MAP_PSO_STATUS_UNINSTALLED,
    SKR_PSO_MAP_PSO_STATUS_COUNT,
    SKR_PSO_MAP_PSO_STATUS_MAX_ENUM_BIT = 0x7FFFFFFF
} ESkrPSOMapPSOStatus;

// create a pso map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
skr_pso_map_id skr_pso_map_create(const struct skr_pso_map_root_t* desc);

// (RC) create a pso map key
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
skr_pso_map_key_id skr_pso_map_create_key(skr_pso_map_id map, const struct CGPURenderPipelineDescriptor* desc);

// (RC) free a pso map key
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_pso_map_free_key(skr_pso_map_id map, skr_pso_map_key_id key);

// (RC) request a pso install
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
ESkrPSOMapPSOStatus skr_pso_map_install_pso(skr_pso_map_id mao, skr_pso_map_key_id key);

// (RC) request a pso uninstall
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_pso_map_uninstall_pso(skr_pso_map_id mao, skr_pso_map_key_id key);

// pso map new frame
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_pso_map_new_frame(skr_pso_map_id psoMap, uint64_t frame_index);

// thread-safe.
// do (RC) garbage collect once for pso map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_pso_map_garbage_collect(skr_pso_map_id psoMap, uint64_t critical_frame);

// free a pso map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_pso_map_free(skr_pso_map_id pso_map);

typedef struct skr_pso_map_root_t {
    SRenderDeviceId render_device = nullptr;
    skr_threaded_service_t* aux_service = nullptr;
} skr_pso_map_root_t;

#ifdef __cplusplus
struct SKR_RENDERER_API skr_pso_map_t
{    
    virtual ~skr_pso_map_t() = default;

    virtual skr_pso_map_key_id create_key(const struct CGPURenderPipelineDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual void free_key(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;
    virtual ESkrPSOMapPSOStatus install_pso(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;
    virtual void uninstall_pso(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;

    virtual void new_frame(uint64_t frame_index) SKR_NOEXCEPT = 0;
    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT = 0;

    static skr_pso_map_id Create(const struct skr_pso_map_root_t* desc) SKR_NOEXCEPT;
    static bool Free(skr_pso_map_id pso_map) SKR_NOEXCEPT;
};
#endif