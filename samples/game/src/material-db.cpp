#include "render-scene.h"
#include "gamert.h"
#include "utils/make_zeroed.hpp"
#include <EASTL/unordered_map.h>
#include <EASTL/vector_map.h>
#include <EASTL/string.h>

static struct 
{
    eastl::unordered_map<gfx_material_id_t, 
        eastl::vector_map<eastl::string, dual_type_index_t>> bindingTypeDB;
} matDB;

dual_type_index_t ecsr_query_material_parameter_type(gfx_material_id_t mat_id, const char8_t* name)
{
    auto&& db = matDB.bindingTypeDB[mat_id];
    if (name != nullptr)
    {
        auto type = db.find(name);
        if(type != db.end())
        {
            return type->second;
        }
        return UINT32_MAX;
    }
    // allocate all
    dual_storage_t* storage = gamert_get_ecs_world();
    auto view = make_zeroed<dual_chunk_view_t>();
    dualS_access(storage, mat_id, &view);
    CGPURootSignatureId RS = ((CGPURootSignatureId*)dualV_get_owned_rw(&view, gfx_root_sig_type))[0];
    // make or check the register table of this material
    for (uint32_t i = 0; i < RS->table_count; i++)
    {
        for (uint32_t j = 0; j < RS->tables[i].resources_count; j++)
        {
            CGPUShaderResource* res = &RS->tables[i].resources[j];
            if (ecsr_query_material_parameter_type(mat_id, res->name) == UINT32_MAX)
            {
                auto type_desc = make_zeroed<dual_type_description_t>();
                eastl::string unique_string = eastl::to_string(mat_id).append(res->name);
                type_desc.name = unique_string.c_str();
                type_desc.size = res->size;
                dual_make_guid(&type_desc.guid);
                db[res->name] = dualT_register_type(&type_desc);
            }
        }
    }
    for (uint32_t i = 0; i < RS->push_constant_count; i++)
    {
        CGPUShaderResource* res = &RS->push_constants[i];
        if (ecsr_query_material_parameter_type(mat_id, res->name) == UINT32_MAX)
        {
            auto type_desc = make_zeroed<dual_type_description_t>();
            eastl::string unique_string = eastl::to_string(mat_id).append("-").append(res->name);
            type_desc.name = unique_string.c_str();
            type_desc.size = res->size;
            dual_make_guid(&type_desc.guid);
            db[res->name] = dualT_register_type(&type_desc);
        }
    }
    return 0;
}