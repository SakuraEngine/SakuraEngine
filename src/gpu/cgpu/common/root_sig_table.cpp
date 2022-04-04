#include "cgpu/api.h"
#include "cgpu/flags.h"
#include "common_utils.h"
#include <EASTL/vector.h>
#include <EASTL/set.h>

extern "C" {
bool CGpuUtil_ShaderResourceIsStaticSampler(CGpuShaderResource* resource, const struct CGpuRootSignatureDescriptor* desc)
{
    for (uint32_t i = 0; i < desc->static_sampler_count; i++)
    {
        if (strcmp(resource->name, desc->static_sampler_names[i]) == 0)
        {
            return resource->type == RT_SAMPLER;
        }
    }
    return false;
}

bool CGpuUtil_ShaderResourceIsRootConst(CGpuShaderResource* resource, const struct CGpuRootSignatureDescriptor* desc)
{
    if (resource->type == RT_ROOT_CONSTANT) return true;
    for (uint32_t i = 0; i < desc->root_constant_count; i++)
    {
        if (strcmp(resource->name, desc->root_constant_names[i]) == 0)
            return true;
    }
    return false;
}

char8_t* duplicate_string(const char8_t* src_string)
{
    if (src_string != CGPU_NULLPTR)
    {
        const size_t source_len = strlen(src_string);
        char8_t* result = (char8_t*)cgpu_malloc(sizeof(char8_t) * (1 + source_len));
#ifdef _WIN32
        strcpy_s((char8_t*)result, source_len + 1, src_string);
#else
        strcpy((char8_t*)result, src_string);
#endif
        return result;
    }
    return CGPU_NULLPTR;
}

// 这是一个非常复杂的过程，牵扯到大量的move和join操作。具体的逻辑如下：
// 1.收集所有ShaderStage中出现的所有ShaderResource，不管他们是否重复
//   这个阶段也会收集所有的RootConst，并且对它们进行合并（不同阶段出现的相同RootConst的Stage合并）
//   也会收集所有的StaticSamplers
// 2.合并ShaderResources到RootSignatureTable（下称为RST）中
//   拥有相同set、binding以及type的、出现在不同ShaderStage中的ShaderResource会被合并Stage
// 3.切分行
//   按照Set把合并好的Resource分割并放进roosting::tables中
void CGpuUtil_InitRSParamTables(CGpuRootSignature* RS, const struct CGpuRootSignatureDescriptor* desc)
{
    CGpuShaderReflection* entry_reflections[32] = { 0 };
    // Pick shader reflection data
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        const CGpuPipelineShaderDescriptor* shader_desc = &desc->shaders[i];
        // Find shader reflection
        for (uint32_t j = 0; j < shader_desc->library->entrys_count; j++)
        {
            CGpuShaderReflection* temp_entry_reflcetion = &shader_desc->library->entry_reflections[j];
            if (strcmp(shader_desc->entry, temp_entry_reflcetion->entry_name) == 0)
            {
                entry_reflections[i] = temp_entry_reflcetion;
                break;
            }
        }
        if (entry_reflections[i] == CGPU_NULLPTR)
        {
            entry_reflections[i] = &shader_desc->library->entry_reflections[0];
        }
    }
    // Collect all resources
    RS->pipeline_type = PIPELINE_TYPE_NONE;
    eastl::vector<CGpuShaderResource> all_resources;
    eastl::vector<CGpuShaderResource> all_root_constants;
    eastl::vector<CGpuShaderResource> all_static_samplers;
    for (uint32_t i = 0; i < desc->shader_count; i++)
    {
        CGpuShaderReflection* reflection = entry_reflections[i];
        for (uint32_t j = 0; j < reflection->shader_resources_count; j++)
        {
            CGpuShaderResource& resource = reflection->shader_resources[j];
            if (CGpuUtil_ShaderResourceIsRootConst(&resource, desc))
            {
                bool coincided = false;
                for (auto&& root_const : all_root_constants)
                {
                    if (root_const.name_hash == resource.name_hash &&
                        root_const.set == resource.set &&
                        root_const.binding == resource.binding &&
                        root_const.size == resource.size)
                    {
                        root_const.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_root_constants.emplace_back(resource);
            }
            else if (CGpuUtil_ShaderResourceIsStaticSampler(&resource, desc))
            {
                bool coincided = false;
                for (auto&& static_sampler : all_static_samplers)
                {
                    if (static_sampler.name_hash == resource.name_hash &&
                        static_sampler.set == resource.set &&
                        static_sampler.binding == resource.binding)
                    {
                        static_sampler.stages |= resource.stages;
                        coincided = true;
                    }
                }
                if (!coincided)
                    all_static_samplers.emplace_back(resource);
            }
            else
            {
                all_resources.emplace_back(resource);
            }
        }
        // Pipeline Type
        if (reflection->stage & SHADER_STAGE_COMPUTE)
            RS->pipeline_type = PIPELINE_TYPE_COMPUTE;
        else if (reflection->stage & SHADER_STAGE_RAYTRACING)
            RS->pipeline_type = PIPELINE_TYPE_RAYTRACING;
        else
            RS->pipeline_type = PIPELINE_TYPE_GRAPHICS;
    }
    // Merge
    eastl::set<uint32_t> valid_sets;
    eastl::vector<CGpuShaderResource> RST_resources;
    RST_resources.reserve(all_resources.size());
    for (auto&& shader_resource : all_resources)
    {
        bool coincided = false;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == shader_resource.set &&
                RST_resource.binding == shader_resource.binding &&
                RST_resource.type == shader_resource.type)
            {
                RST_resource.stages |= shader_resource.stages;
                coincided = true;
            }
        }
        if (!coincided)
        {
            valid_sets.insert(shader_resource.set);
            RST_resources.emplace_back(shader_resource);
        }
    }
    // Slice
    RS->table_count = valid_sets.size();
    RS->tables = (CGpuParameterTable*)cgpu_calloc(RS->table_count, sizeof(CGpuParameterTable));
    uint32_t table_index = 0;
    for (auto set_index : valid_sets)
    {
        CGpuParameterTable& table = RS->tables[table_index];
        table.set_index = set_index;
        table.resources_count = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
                table.resources_count++;
        }
        table.resources = (CGpuShaderResource*)cgpu_calloc(
            table.resources_count, sizeof(CGpuShaderResource));
        uint32_t slot_index = 0;
        for (auto&& RST_resource : RST_resources)
        {
            if (RST_resource.set == set_index)
            {
                table.resources[slot_index] = RST_resource;
                slot_index++;
            }
        }
        table_index++;
    }
    // push constants
    RS->push_constant_count = all_root_constants.size();
    RS->push_constants = (CGpuShaderResource*)cgpu_calloc(
        RS->push_constant_count, sizeof(CGpuShaderResource));
    for (uint32_t i = 0; i < all_root_constants.size(); i++)
    {
        RS->push_constants[i] = all_root_constants[i];
    }
    // static samplers
    RS->static_sampler_count = all_static_samplers.size();
    RS->static_samplers = (CGpuShaderResource*)cgpu_calloc(
        RS->static_sampler_count, sizeof(CGpuShaderResource));
    for (uint32_t i = 0; i < all_static_samplers.size(); i++)
    {
        RS->static_samplers[i] = all_static_samplers[i];
    }
    // copy names
    for (uint32_t i = 0; i < RS->push_constant_count; i++)
    {
        CGpuShaderResource* dst = RS->push_constants + i;
        dst->name = duplicate_string(dst->name);
    }
    for (uint32_t i = 0; i < RS->static_sampler_count; i++)
    {
        CGpuShaderResource* dst = RS->static_samplers + i;
        dst->name = duplicate_string(dst->name);
    }
    for (uint32_t i = 0; i < RS->table_count; i++)
    {
        CGpuParameterTable* set_to_record = &RS->tables[i];
        for (uint32_t j = 0; j < set_to_record->resources_count; j++)
        {
            CGpuShaderResource* dst = &set_to_record->resources[j];
            dst->name = duplicate_string(dst->name);
        }
    }
}

void CGpuUtil_FreeRSParamTables(CGpuRootSignature* RS)
{
    if (RS->tables != CGPU_NULLPTR)
    {
        for (uint32_t i_set = 0; i_set < RS->table_count; i_set++)
        {
            CGpuParameterTable* param_table = &RS->tables[i_set];
            if (param_table->resources != CGPU_NULLPTR)
            {
                for (uint32_t i_binding = 0; i_binding < param_table->resources_count; i_binding++)
                {
                    CGpuShaderResource* binding_to_free = &param_table->resources[i_binding];
                    if (binding_to_free->name != CGPU_NULLPTR)
                    {
                        cgpu_free((char8_t*)binding_to_free->name);
                    }
                }
                cgpu_free(param_table->resources);
            }
        }
        cgpu_free(RS->tables);
    }
    if (RS->push_constants != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < RS->push_constant_count; i++)
        {
            CGpuShaderResource* binding_to_free = RS->push_constants + i;
            if (binding_to_free->name != CGPU_NULLPTR)
            {
                cgpu_free((char8_t*)binding_to_free->name);
            }
        }
        cgpu_free(RS->push_constants);
    }
    if (RS->static_samplers != CGPU_NULLPTR)
    {
        for (uint32_t i = 0; i < RS->static_sampler_count; i++)
        {
            CGpuShaderResource* binding_to_free = RS->static_samplers + i;
            if (binding_to_free->name != CGPU_NULLPTR)
            {
                cgpu_free((char8_t*)binding_to_free->name);
            }
        }
        cgpu_free(RS->static_samplers);
    }
}
}