#pragma once
#include "platform/configure.h"
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/fwd_types.h"
#include "cgpu/api.h"

SKR_DECLARE_TYPE_ID_FWD(skr, JobQueue, skr_job_queue)

typedef enum ESkrPSOMapPSOStatus
{
    SKR_PSO_MAP_PSO_STATUS_REQUESTED = 1,
    SKR_PSO_MAP_PSO_STATUS_FAILED = 2,
    SKR_PSO_MAP_PSO_STATUS_INSTALLED = 3,
    SKR_PSO_MAP_PSO_STATUS_UNINSTALLED = 4
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
ESkrPSOMapPSOStatus skr_pso_map_install_pso(skr_pso_map_id map, skr_pso_map_key_id key);

// thread-safe.
// fetch pso in pso map
SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
CGPURenderPipelineId skr_pso_map_find_pso(skr_pso_map_id map, skr_pso_map_key_id key);

// (RC) request a pso uninstall
SKR_RENDERER_EXTERN_C SKR_RENDERER_API
bool skr_pso_map_uninstall_pso(skr_pso_map_id map, skr_pso_map_key_id key);

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
    CGPUDeviceId device SKR_IF_CPP(= nullptr);
    skr_job_queue_id job_queue SKR_IF_CPP(= nullptr;)
} skr_pso_map_root_t;

#ifdef __cplusplus
struct SKR_RENDERER_API skr_pso_map_t
{    
    virtual ~skr_pso_map_t() = default;

    virtual skr_pso_map_key_id create_key(const struct CGPURenderPipelineDescriptor* desc) SKR_NOEXCEPT = 0;
    virtual void free_key(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;
    virtual ESkrPSOMapPSOStatus install_pso(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;
    virtual CGPURenderPipelineId find_pso(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;
    virtual bool uninstall_pso(skr_pso_map_key_id key) SKR_NOEXCEPT = 0;

    virtual void new_frame(uint64_t frame_index) SKR_NOEXCEPT = 0;
    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT = 0;

    static skr_pso_map_id Create(const struct skr_pso_map_root_t* desc) SKR_NOEXCEPT;
    static bool Free(skr_pso_map_id pso_map) SKR_NOEXCEPT;
};
#endif