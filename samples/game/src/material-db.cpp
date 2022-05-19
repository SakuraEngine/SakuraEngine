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

bool ecsr_renderable_primitive_type(const skr_scene_primitive_desc_t* desc, 
dual_type_index_t* ctypes, uint32_t* ctype_count, dual_entity_t* emetas, uint32_t* meta_count)
{
    *meta_count = 0;
    *ctype_count = 0;
    if (desc->material)
    {
        *meta_count = 1;
        if(emetas != NULL) emetas[0] = desc->material;
        *ctype_count += 4;
        if(ctypes)
        {
            ctypes[0] = gfx_material_type;
            ctypes[1] = transform_type;
            ctypes[2] = vertex_buffer_type;
            ctypes[3] = index_buffer_type;
        }
        const uint32_t bcount = matDB.bindingTypeDB[desc->material].size();
        if (ctypes)
        {
            for(uint32_t i = 0; i < bcount; i++)
            {
                ctypes[*ctype_count + i] = matDB.bindingTypeDB[desc->material].at(i).second;
            }
        }
        *ctype_count += bcount;
        return true;
    }
    return false;
}