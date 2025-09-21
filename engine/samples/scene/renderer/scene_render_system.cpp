#include "SkrBase/math.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "scene_render_system.h"
#include "SkrAnim/components/skin_component.hpp"

#include "SkrCore/log.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "SkrSceneCore/scene_components.h"
#include "SkrRenderer/render_mesh.h"
#include "helper.hpp"
#include "scene_renderer.hpp"
#include "SkrRenderer/resources/texture_resource.h"
#include "SkrRenderer/resources/material_resource.hpp"

namespace skr
{
namespace render_graph
{
class RenderGraph;
}
} // namespace skr

namespace skr::scene
{
struct SceneRenderJob
{
    using RenderF = std::function<void(const skr::MeshResource*, skr_float4x4_t, const skr::AnimComponent*)>;
    SceneRenderJob(RenderF render_callback = nullptr)
        : render_callback(render_callback)
    {
    }

    void build(skr::ecs::AccessBuilder& builder)
    {
        builder.has<MeshComponent>()
            .has<scene::TransformComponent>();

        builder.access(&SceneRenderJob::mesh_accessor)
            .access(&SceneRenderJob::transform_accessor)
            .access(&SceneRenderJob::anim_accessor);
    }
    void run(skr::ecs::TaskContext& context)
    {
        SkrZoneScopedN("SceneRenderJob::run");
        for (auto i = 0; i < context.size(); i++)
        {
            const auto entity = context.entities()[i];
            // SKR_LOG_INFO(u8"Rendering entity: {%u}", entity);
            auto* mesh_component = mesh_accessor.get(entity);
            auto* transform_component = transform_accessor.get(entity);
            auto* pAnimComponent = anim_accessor.get(entity);
            // SKR_LOG_INFO(u8"pAnimComponent: %p", pAnimComponent);

            if (!mesh_component || !transform_component)
            {
                // not a renderable entity
                continue;
            }

            mesh_component->mesh_resource.resolve(true, 0);
            if (mesh_component->mesh_resource.is_resolved())
            {
                skr::MeshResource* mesh_resource = mesh_component->mesh_resource.get_resolved(true);
                for (auto& mat : mesh_resource->materials)
                {
                    mat.resolve(true, 0);
                    if (mat.is_resolved())
                    {
                        skr::MaterialResource* mat_resource = mat.get_resolved(true);
                        for (auto& tex : mat_resource->overrides.textures)
                        {
                            skr::AsyncResource<skr::TextureResource> tex_handle = tex.value;
                            tex_handle.resolve(true, 0);
                        }
                    }
                }
                if (mesh_resource && mesh_resource->render_mesh)
                {
                    render_callback(mesh_resource, transform_component->get().to_matrix(), pAnimComponent);
                }
                else
                {
                    continue;
                }
            }
        }
    }

    skr::ecs::RandomComponentReadWrite<skr::MeshComponent> mesh_accessor;
    skr::ecs::RandomComponentReader<const skr::scene::TransformComponent> transform_accessor;
    skr::ecs::RandomComponentReader<const skr::AnimComponent> anim_accessor;

    RenderF render_callback = nullptr;
    bool with_anim = false;
};

struct SceneRenderSystem::Impl
{
    skr::ecs::ECSWorld* mp_world = nullptr;
    sugoi_query_t* m_render_job_query = nullptr;
    skr::SceneRenderer* mp_renderer = nullptr;
    struct PushConstants
    {
        skr_float4x4_t model = skr_float4x4_t::identity();
        skr_float4x4_t view_proj = skr_float4x4_t::identity();
        bool use_base_color_texture = false;
    };
    skr::Vector<skr_primitive_draw_t> drawcalls;
    skr::Vector<PushConstants> push_constants_list;
    SceneRenderSystem::Context context;

    skr::Map<CGPUTextureViewId, CGPUXBindTableId> bind_tables;
    const char8_t* color_texture_name = u8"color_texture";
};

SceneRenderSystem* SceneRenderSystem::Create(skr::ecs::ECSWorld* world) SKR_NOEXCEPT
{
    SkrZoneScopedN("CreateSceneRenderSystem");
    auto memory = (uint8_t*)sakura_calloc(1, sizeof(SceneRenderSystem) + sizeof(SceneRenderSystem::Impl));
    // placement new for constructing the SceneRenderSystem and its Impl compactly
    auto system = new (memory) SceneRenderSystem();
    system->impl = new (memory + sizeof(SceneRenderSystem)) SceneRenderSystem::Impl();
    system->impl->mp_world = world;
    return system;
};

void SceneRenderSystem::Destroy(SceneRenderSystem* system) SKR_NOEXCEPT
{
    SkrZoneScopedN("DestroySceneRenderSystem");
    system->impl->~Impl();
    system->~SceneRenderSystem();
    sakura_free(system);
}

void SceneRenderSystem::bind_renderer(skr::SceneRenderer* renderer) SKR_NOEXCEPT
{
    impl->mp_renderer = renderer;
}

SceneRenderSystem::Context const* SceneRenderSystem::get_context() const SKR_NOEXCEPT
{
    return &impl->context;
}

skr::Span<skr_primitive_draw_t> SceneRenderSystem::get_drawcalls() const SKR_NOEXCEPT
{
    return impl->drawcalls;
}

void SceneRenderSystem::update() SKR_NOEXCEPT
{
    SkrZoneScopedN("SceneRenderSystem::update");
    impl->drawcalls.clear();
    impl->push_constants_list.clear();

    skr::ecs::TaskOptions options;
    impl->context.update_finish.clear();
    options.on_finishes.add(impl->context.update_finish);

    auto render_func = impl->mp_renderer != nullptr ?
        skr::scene::SceneRenderJob::RenderF([this](const skr::MeshResource* mesh, skr_float4x4_t model, const skr::AnimComponent* pAnimComponent) {
            auto pso = impl->mp_renderer->get_render_pso();
            auto& push_constants_data = impl->push_constants_list.emplace().ref();
            push_constants_data.model = skr::transpose(model);
            utils::Camera* camera = impl->mp_renderer->get_camera();
            auto view = skr::float4x4::view_at(
                camera->pos,
                camera->pos + camera->front,
                camera->up);
            auto proj = skr::float4x4::perspective_fov(
                skr::camera_fov_y_from_x(camera->fov, camera->aspect),
                camera->aspect,
                camera->near_plane,
                camera->far_plane);
            auto _view_proj = skr::mul(view, proj);
            push_constants_data.view_proj = skr::transpose(_view_proj);
            auto& cmds = mesh->render_mesh->primitive_commands;

            for (auto i = 0; i < cmds.size(); i++)
            {
                auto& cmd = cmds[i];
                skr_primitive_draw_t& drawcall = impl->drawcalls.emplace().ref();
                if (mesh->materials.size() > cmd.material_index)
                {
                    auto& mat_handle = mesh->materials[cmd.material_index];
                    if (mat_handle.is_resolved())
                    {
                        skr::MaterialResource* mat_resource = mat_handle.get_resolved(true);
                        for (auto& tex : mat_resource->overrides.textures)
                        {
                            if (skr::String(tex.slot_name) == u8"BaseColor")
                            {
                                skr::AsyncResource<skr::TextureResource> tex_handle = tex.value;
                                tex_handle.resolve(true, 0);
                                if (tex_handle.is_resolved())
                                {
                                    skr::TextureResource* tex_resource = tex_handle.get_resolved(true);
                                    // SKR_LOG_INFO(u8"Texture %s resolved with id %u", tex.slot_name.c_str(), tex_resource->texture);
                                    auto& tex_view = tex_resource->texture_view;
                                    {
                                        if (!impl->bind_tables.contains(tex_view))
                                        {
                                            SkrZoneScopedN("SOCRender::createBindTable");
                                            CGPUXBindTableDescriptor bind_table_desc = {};
                                            bind_table_desc.root_signature = pso->root_signature;
                                            bind_table_desc.names = &impl->color_texture_name;
                                            bind_table_desc.names_count = 1;

                                            auto bind_table = cgpux_create_bind_table(pso->device, &bind_table_desc);

                                            impl->bind_tables.add(tex_view, bind_table);

                                            CGPUDescriptorData datas[1] = {};
                                            datas[0] = make_zeroed<CGPUDescriptorData>();
                                            datas[0].by_name.name = impl->color_texture_name;
                                            datas[0].count = 1;
                                            datas[0].textures = &tex_view;
                                            cgpux_bind_table_update(bind_table, datas, 1);
                                        }
                                        drawcall.bind_table = impl->bind_tables.find(tex_view).value();
                                        if (drawcall.bind_table)
                                        {
                                            push_constants_data.use_base_color_texture = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                drawcall.vertex_buffer_count = (uint32_t)cmd.vbvs.size();
                if (pAnimComponent)
                {
                    auto& skin_prim = pAnimComponent->primitives[i];
                    drawcall.vertex_buffers = skin_prim.views.data();
                }
                else
                {
                    drawcall.vertex_buffers = cmd.vbvs.data();
                }
                drawcall.index_buffer = *cmd.ibv;
                drawcall.push_const = (const uint8_t*)(&push_constants_data);
            }
        }) :
        skr::scene::SceneRenderJob::RenderF([](const skr::MeshResource*, skr_float4x4_t, const skr::AnimComponent*) {
            // do nothing
        });
    scene::SceneRenderJob job{ render_func };
    impl->m_render_job_query = impl->mp_world->dispatch_task(job, UINT32_MAX, impl->m_render_job_query, std::move(options));
}

} // namespace skr::scene
