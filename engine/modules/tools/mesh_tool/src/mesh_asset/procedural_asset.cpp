#include "SkrBase/misc/defer.hpp"
#include "SkrCore/log.hpp"
#include "SkrRTTR/type_registry.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrToolCore/cook_system/cook_system.hpp"
#include "SkrToolCore/project/project.hpp"
#include "SkrMeshTool/mesh_asset.hpp"
#include "SkrMeshTool/mesh_processing.hpp"
#include "MeshOpt/meshoptimizer.h"
#include <SkrRTTR/type_registry.hpp>

#include "SkrProfile/profile.h"
#include "SkrRTTR/type.hpp"
#include "SkrMeshCore/mesh_processing.hpp"
#include "SkrContainersDef/stl_string.hpp"
#include "SkrGraphics/api.h"

namespace skd::asset
{

void* ProceduralMeshImporter::Import(skr::io::IRAMService* ioService, CookContext* context)
{
    skr::RTTRType* type = get_type_from_guid(built_in_mesh_tid);
    auto assetMetaFile = context->GetAssetMetaFile();
    auto& mesh_asset = *assetMetaFile->GetMetadata<MeshAsset>();

    if (!type)
    {
        SKR_LOG_FMT_FATAL(u8"BuiltinMeshImporter: Failed to find type for guid {}", built_in_mesh_tid);
        return nullptr;
    }
    void* data = sakura_malloc_aligned(type->size(), type->alignment());
    type->find_default_ctor().invoke(data);
    ProceduralMesh* mesh = (ProceduralMesh*)data;
    mesh->configure(&mesh_asset);
    return (void*)data;
}

void ProceduralMeshImporter::Destroy(void* resource)
{
    skr::RTTRType* type = get_type_from_guid(built_in_mesh_tid);
    sakura_free_aligned((ProceduralMesh*)resource, type->alignment());
    return;
}

void SimpleTriangleMesh::generate_resource(skr::MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins, GUID shuffle_layout_id)
{
    // TODO: directly construct on bins, no copy
    skr::Vector<uint8_t> buffer0 = {};
    out_resource.name = u8"BuiltInSimpleTriangleMesh";
    auto& mesh_section = out_resource.sections.add_default().ref();
    mesh_section.translation = { 0.0f, 0.0f, 0.0f };
    mesh_section.scale = { 1.0f, 1.0f, 1.0f };
    mesh_section.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };

    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }
    uint32_t indices[] = { 0, 1, 2 };
    skr::Vector<float3> c_positions;
    skr::Vector<float2> c_uvs;
    skr::Vector<float3> c_normals;
    skr::Vector<uint32_t> c_indices;
    c_positions.push_back({ -1.0f, -0.5f, 0.0f });
    c_positions.push_back({ 1.0f, -1.0f, 0.0f });
    c_positions.push_back({ 0.0f, 1.0f, 0.0f });

    c_uvs.push_back({ 0.0f, 1.0f });
    c_uvs.push_back({ 1.0f, 1.0f });
    c_uvs.push_back({ 0.5f, 0.0f });

    c_normals.push_back({ 0.0f, 0.0f, 1.0f });
    c_normals.push_back({ 0.0f, 0.0f, 1.0f });
    c_normals.push_back({ 0.0f, 0.0f, 1.0f });

    c_indices.append({ 0, 1, 2 });

    SRawMesh raw_mesh = {};
    raw_mesh.primitives.reserve(1);
    auto& raw_prim = raw_mesh.primitives.add_default().ref();
    // fill indices
    {
        raw_prim.index_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_indices.data(), sizeof(c_indices));
        raw_prim.index_stream.offset = 0;
        raw_prim.index_stream.count = c_indices.size();
        raw_prim.index_stream.stride = sizeof(uint32_t);
    }
    // fill vertex stream
    raw_prim.vertex_streams.reserve(2);
    {
        auto& vertex_stream = raw_prim.vertex_streams.add_default().ref();
        vertex_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_positions.data(), sizeof(c_positions));
        vertex_stream.offset = 0;
        vertex_stream.count = c_positions.size();
        vertex_stream.stride = sizeof(float) * 3;
        vertex_stream.type = ERawVertexStreamType::POSITION;

        auto& texcoord_stream = raw_prim.vertex_streams.add_default().ref();
        texcoord_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_uvs.data(), sizeof(c_uvs));
        texcoord_stream.offset = 0;
        texcoord_stream.count = c_uvs.size();
        texcoord_stream.stride = sizeof(float) * 2;
        texcoord_stream.type = ERawVertexStreamType::TEXCOORD;
    }

    skr::Vector<MeshPrimitive> new_primitives;
    EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
    EmplaceAllRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
    SKR_ASSERT(new_primitives.size() == 1 && "SimpleTriangleMesh should have only one primitive");
    auto& prim0 = new_primitives[0];
    prim0.vertex_layout = shuffle_layout_id;
    prim0.material_index = 0; // no material
    mesh_section.primitive_indices.add(out_resource.primitives.size());
    out_resource.primitives.reserve(1);
    out_resource.primitives += new_primitives;

    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }

    out_bins.add(buffer0);
}

void SimpleCubeMesh::generate_resource(skr::MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins, GUID shuffle_layout_id)
{
    skr::Vector<uint8_t> buffer0 = {};
    out_resource.name = u8"BuiltInSimpleCubeMesh";
    auto& mesh_section = out_resource.sections.add_default().ref();
    mesh_section.translation = { 0.0f, 0.0f, 0.0f };
    mesh_section.scale = { 1.0f, 1.0f, 1.0f };
    mesh_section.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };

    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }
    // clang-format off
    float size = 1.0f;
    float half_size = size * 0.5f;
    skr::Vector<float3> c_positions;
    skr::Vector<float2> c_uvs;
    skr::Vector<float3> c_normals;
    skr::Vector<uint16_t> c_indices;
    // 8 vertices of a cube
    c_positions.push_back({ -half_size, -half_size, -half_size }); // 0: left-bottom-back
    c_positions.push_back({ half_size, -half_size, -half_size });  // 1: right-bottom-back
    c_positions.push_back({ half_size, half_size, -half_size });   // 2: right-top-back
    c_positions.push_back({ -half_size, half_size, -half_size });  // 3: left-top-back
    c_positions.push_back({ -half_size, -half_size, half_size });  // 4: left-bottom-front
    c_positions.push_back({ half_size, -half_size, half_size });   // 5: right-bottom-front
    c_positions.push_back({ half_size, half_size, half_size });    // 6: right-top-front
    c_positions.push_back({ -half_size, half_size, half_size });   // 7: left-top-front

    // UV coordinates for each vertex
    c_uvs.push_back({ 0.0f, 0.0f });
    c_uvs.push_back({ 1.0f, 0.0f });
    c_uvs.push_back({ 1.0f, 1.0f });
    c_uvs.push_back({ 0.0f, 1.0f });
    c_uvs.push_back({ 0.0f, 0.0f });
    c_uvs.push_back({ 1.0f, 0.0f });
    c_uvs.push_back({ 1.0f, 1.0f });
    c_uvs.push_back({ 0.0f, 1.0f });

    // Normals for each vertex (cube has 6 faces, but we'll use averaged normals)
    c_normals.push_back({ -0.577f, -0.577f, -0.577f }); // normalized diagonal
    c_normals.push_back({ 0.577f, -0.577f, -0.577f });
    c_normals.push_back({ 0.577f, 0.577f, -0.577f });
    c_normals.push_back({ -0.577f, 0.577f, -0.577f });
    c_normals.push_back({ -0.577f, -0.577f, 0.577f });
    c_normals.push_back({ 0.577f, -0.577f, 0.577f });
    c_normals.push_back({ 0.577f, 0.577f, 0.577f });
    c_normals.push_back({ -0.577f, 0.577f, 0.577f });

    // 12 triangles (36 indices) for cube faces
    // Back face
    c_indices.append({ 0, 1, 2, 0, 2, 3 });
    c_indices.append({ 4, 6, 5, 4, 7, 6 });
    c_indices.append({ 0, 3, 7, 0, 7, 4 });
    c_indices.append({ 1, 5, 6, 1, 6, 2 });
    c_indices.append({ 0, 4, 5, 0, 5, 1 });
    c_indices.append({ 3, 2, 6, 3, 6, 7 });

    // clang-format on
    SRawMesh raw_mesh = {};
    raw_mesh.primitives.reserve(1);
    auto& raw_prim = raw_mesh.primitives.add_default().ref();
    // fill indices
    {
        raw_prim.index_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_indices.data(), sizeof(c_indices));
        raw_prim.index_stream.offset = 0;
        raw_prim.index_stream.count = c_indices.size();
        raw_prim.index_stream.stride = sizeof(uint16_t);
    }
    // fill vertex stream
    {
        raw_prim.vertex_streams.reserve(2);

        auto& vertex_stream = raw_prim.vertex_streams.add_default().ref();
        vertex_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_positions.data(), sizeof(c_positions));
        vertex_stream.offset = 0;
        vertex_stream.count = c_positions.size();
        vertex_stream.stride = sizeof(float) * 3;
        vertex_stream.type = ERawVertexStreamType::POSITION;

        auto& texcoord_stream = raw_prim.vertex_streams.add_default().ref();
        texcoord_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_uvs.data(), sizeof(c_uvs));
        texcoord_stream.offset = 0;
        texcoord_stream.count = c_uvs.size();
        texcoord_stream.stride = sizeof(float) * 2;
        texcoord_stream.type = ERawVertexStreamType::TEXCOORD;
    }

    skr::Vector<MeshPrimitive> new_primitives;
    EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
    EmplaceAllRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
    SKR_ASSERT(new_primitives.size() == 1 && "SimpleTriangleMesh should have only one primitive");
    auto& prim0 = new_primitives[0];
    prim0.vertex_layout = shuffle_layout_id;
    prim0.material_index = 0; // no material
    mesh_section.primitive_indices.add(out_resource.primitives.size());
    out_resource.primitives.reserve(1);
    out_resource.primitives += new_primitives;

    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }

    out_bins.add(buffer0);
}

void SimpleGridMesh::configure(const MeshAsset* args)
{
    if (args)
    {
        auto pArgs = static_cast<const SimpleGridMeshAsset*>(args);
        x_segments = pArgs->x_segments;
        y_segments = pArgs->y_segments;
        x_size = pArgs->x_size;
        y_size = pArgs->y_size;
    }
}
void SimpleGridMesh::generate_resource(skr::MeshResource& out_resource, skr::Vector<skr::Vector<uint8_t>>& out_bins, GUID shuffle_layout_id)
{
    skr::Vector<float3> c_positions;
    skr::Vector<float2> c_uvs;
    skr::Vector<float3> c_normals;
    skr::Vector<uint32_t> c_indices;
    // Calculate total vertices and indices
    int vertices_width = x_segments + 1;
    int vertices_height = y_segments + 1;
    int num_tiles_width = x_segments;
    int num_tiles_height = y_segments;
    // Generate vertices
    for (int y = 0; y < vertices_height; ++y)
    {
        for (int x = 0; x < vertices_width; ++x)
        {
            // Position
            float pos_x = x * x_size - (x_segments * x_size * 0.5f);
            float pos_z = y * y_size - (y_segments * y_size * 0.5f);
            c_positions.push_back({ pos_x, 0.0f, pos_z });

            // UV coordinates
            float u = (float)x / (float)num_tiles_width;
            float v = (float)y / (float)num_tiles_height;
            c_uvs.push_back({ u, v });

            // Normal (pointing up for a flat grid)
            c_normals.push_back({ 0.0f, 1.0f, 0.0f });
        }
    }

    // Generate indices (two triangles per tile)
    for (int y = 0; y < num_tiles_height; ++y)
    {
        for (int x = 0; x < num_tiles_width; ++x)
        {
            int bottom_left = y * vertices_width + x;
            int bottom_right = bottom_left + 1;
            int top_left = bottom_left + vertices_width;
            int top_right = top_left + 1;

            // First triangle (bottom-left, bottom-right, top-left)
            c_indices.append({ static_cast<uint32_t>(bottom_left),
                static_cast<uint32_t>(bottom_right),
                static_cast<uint32_t>(top_left) });

            // Second triangle (bottom-right, top-right, top-left)
            c_indices.append({ static_cast<uint32_t>(bottom_right),
                static_cast<uint32_t>(top_right),
                static_cast<uint32_t>(top_left) });
        }
    }

    skr::Vector<uint8_t> buffer0 = {};
    out_resource.name = u8"BuiltInSimpleGridMesh";
    auto& mesh_section = out_resource.sections.add_default().ref();
    mesh_section.translation = { 0.0f, 0.0f, 0.0f };
    mesh_section.scale = { 1.0f, 1.0f, 1.0f };
    mesh_section.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };

    CGPUVertexLayout shuffle_layout = {};
    const char* shuffle_layout_name = nullptr;
    if (!shuffle_layout_id.is_zero())
    {
        shuffle_layout_name = skr_mesh_resource_query_vertex_layout(shuffle_layout_id, &shuffle_layout);
    }
    // clang-format on
    SRawMesh raw_mesh = {};
    raw_mesh.primitives.reserve(1);
    auto& raw_prim = raw_mesh.primitives.add_default().ref();
    // fill indices
    {
        raw_prim.index_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_indices.data(), sizeof(c_indices));
        raw_prim.index_stream.offset = 0;
        raw_prim.index_stream.count = c_indices.size();
        raw_prim.index_stream.stride = sizeof(uint32_t);
    }
    // fill vertex stream
    {
        raw_prim.vertex_streams.reserve(4);

        auto& vertex_stream = raw_prim.vertex_streams.add_default().ref();
        vertex_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_positions.data(), sizeof(c_positions));
        vertex_stream.offset = 0;
        vertex_stream.count = c_positions.size();
        vertex_stream.stride = sizeof(float) * 3;
        vertex_stream.type = ERawVertexStreamType::POSITION;

        auto& texcoord_stream = raw_prim.vertex_streams.add_default().ref();
        texcoord_stream.buffer_view = skr::Span<const uint8_t>((const uint8_t*)c_uvs.data(), sizeof(c_uvs));
        texcoord_stream.offset = 0;
        texcoord_stream.count = c_uvs.size();
        texcoord_stream.stride = sizeof(float) * 2;
        texcoord_stream.type = ERawVertexStreamType::TEXCOORD;
    }

    skr::Vector<MeshPrimitive> new_primitives;
    EmplaceAllRawMeshIndices(&raw_mesh, buffer0, new_primitives);
    EmplaceAllRawMeshVertices(&raw_mesh, shuffle_layout_name ? &shuffle_layout : nullptr, buffer0, new_primitives);
    SKR_ASSERT(new_primitives.size() == 1 && "SimpleTriangleMesh should have only one primitive");
    auto& prim0 = new_primitives[0];
    prim0.vertex_layout = shuffle_layout_id;
    prim0.material_index = 0; // no material
    mesh_section.primitive_indices.add(out_resource.primitives.size());
    out_resource.primitives.reserve(1);
    out_resource.primitives += new_primitives;

    {
        // record buffer bins
        auto& out_buffer0 = out_resource.bins.add_default().ref();
        out_buffer0.index = 0;
        out_buffer0.byte_length = buffer0.size();
        out_buffer0.used_with_index = true;
        out_buffer0.used_with_vertex = true;
    }

    out_bins.add(buffer0);
}

} // namespace skd::asset
