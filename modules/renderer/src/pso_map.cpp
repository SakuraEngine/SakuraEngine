#include "SkrRenderer/pso_map.h"
#include "SkrRenderer/render_device.h"
#include "platform/atomic.h"
#include "containers/hashmap.hpp"
#include "containers/vector.hpp"
#include "containers/sptr.hpp"
#include "utils/defer.hpp"
#include "utils/format.hpp"
#include "utils/make_zeroed.hpp"
#include "utils/threaded_service.h"

struct skr_pso_map_key_t
{
    CGPURootSignatureId root_signature;
    CGPUPipelineShaderDescriptor vertex_shader;
    skr::vector<CGPUConstantSpecialization> vertex_specializations;
    CGPUPipelineShaderDescriptor tesc_shader;
    skr::vector<CGPUConstantSpecialization> tesc_specializations;
    CGPUPipelineShaderDescriptor tese_shader;
    skr::vector<CGPUConstantSpecialization> tese_specializations;
    CGPUPipelineShaderDescriptor geom_shader;
    skr::vector<CGPUConstantSpecialization> geom_specializations;
    CGPUPipelineShaderDescriptor fragment_shader;
    skr::vector<CGPUConstantSpecialization> fragment_specializations;
    CGPUVertexLayout vertex_layout;
    CGPUBlendStateDescriptor blend_state;
    CGPUDepthStateDescriptor depth_state;
    CGPURasterizerStateDescriptor rasterizer_state;
    ECGPUFormat color_formats[CGPU_MAX_MRT_COUNT];

    CGPURenderPipelineDescriptor descriptor;

    SAtomic64 frame = UINT64_MAX;
    SAtomic32 rc = 0;
    CGPURenderPipelineId pipeline = nullptr;
};

namespace skr
{
struct PSOMapImpl : public skr_pso_map_t
{
    PSOMapImpl(const skr_pso_map_root_t& root)
        : root(root)
    {

    }

    struct key_ptr_equal
    {
        using is_transparent = void;

        size_t operator()(const SPtr<skr_pso_map_key_t>& a, const SPtr<skr_pso_map_key_t>& b) const;
    };

    struct key_ptr_hasher
    {
        using is_transparent = void;

        SKR_RENDERER_API size_t operator()(const SPtr<skr_pso_map_key_t>& key) const;
    };

    virtual skr_pso_map_key_id create_key(const struct CGPURenderPipelineDescriptor* desc) SKR_NOEXCEPT override;
    virtual void free_key(skr_pso_map_key_id key) SKR_NOEXCEPT override;
    virtual ESkrPSOMapPSOStatus install_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override;
    virtual void uninstall_pso(skr_pso_map_key_id key) SKR_NOEXCEPT override;

    virtual void new_frame(uint64_t frame_index) SKR_NOEXCEPT override;
    virtual void garbage_collect(uint64_t critical_frame) SKR_NOEXCEPT override;

    skr_pso_map_root_t root;
    skr::parallel_flat_hash_set<SPtr<skr_pso_map_key_t>, key_ptr_hasher, key_ptr_equal> sets;
    SAtomic64 keys_counter = 0;
};
} // namespace skr

skr_pso_map_id Create(const struct skr_pso_map_root_t* root) SKR_NOEXCEPT
{
    return nullptr;
    // return SkrNew<skr::PSOMapImpl>(*root);
}

bool Free(skr_pso_map_id pso_map) SKR_NOEXCEPT
{
    // SkrDelete(pso_map);
    return true;
}