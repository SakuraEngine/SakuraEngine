#include "SkrCore/module/module_manager.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrTask/fib_task.hpp"
#include "SkrScene/transform_system.hpp"
#include "SkrSystem/system_app.h"
#include "SkrScene/basic_components.hpp"
#include "SkrGraphics/api.h"
#include "SkrCore/log.hpp"
#include "SkrGraphics/raytracing.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include <random>
#include <cmath>
#include <chrono>

class RGRaytracingSampleModule : public skr::IDynamicModule
{
    virtual void on_load(int argc, char8_t** argv) override;
    virtual int main_module_exec(int argc, char8_t** argv) override;
    virtual void on_unload() override;

public:
    static RGRaytracingSampleModule* Get();

    void spawn_entities();
    void create_as();
    void create_sphere_blas();
    void create_SceneTLAS();
    void create_compute_pipeline();
    void create_swapchain(skr::SystemWindow* window);
    void render();

    RGRaytracingSampleModule()
        : world(scheduler)
    {
    }
    skr::SystemApp app;
    skr::ecs::ECSWorld world;
    skr::task::scheduler_t scheduler;
    skr::UPtr<skr::TransformSystem> transform_system = nullptr;

    // Raytracing resources
    CGPUInstanceId instance = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId gfx_queue = nullptr;
    CGPUAccelerationStructureId sphere_blas = nullptr;
    CGPUAccelerationStructureId SceneTLAS = nullptr;
    CGPUBufferId sphere_vertex_buffer = nullptr;
    CGPUBufferId sphere_index_buffer = nullptr;

    // Compute pipeline resources
    CGPUShaderLibraryId compute_shader = nullptr;
    CGPURootSignatureId root_signature = nullptr;
    CGPUComputePipelineId compute_pipeline = nullptr;

    // Swapchain resources
    CGPUSwapChainId swapchain = nullptr;

    // RenderGraph resources
    skr::RG::RenderGraph* render_graph = nullptr;

    // Camera control state
    struct CameraController
    {
        skr::math::float3 position = { -25000.0f, 15000.0f, -35000.0f };
        skr::math::float3 target = { 0.0f, 2000.0f, 0.0f };
        skr::math::float3 up = { 0.0f, 1.0f, 0.0f };

        float yaw = 0.0f;   // 水平旋转角度
        float pitch = 0.0f; // 垂直旋转角度
        float move_speed = 5000.0f;
        float mouse_sensitivity = 0.005f;

        bool right_mouse_pressed = false;
        float last_mouse_x = 0.0f;
        float last_mouse_y = 0.0f;

        // 键盘状态
        bool keys_pressed[256] = { false };

        void update_camera(float delta_time)
        {
            // 根据yaw和pitch计算前方向量
            skr::math::float3 forward = {
                cos(pitch) * cos(yaw),
                sin(pitch),
                cos(pitch) * sin(yaw)
            };
            forward = skr::math::normalize(forward);

            // 计算右方向量和上方向量
            skr::math::float3 right = skr::math::normalize(skr::math::cross(forward, { 0.0f, 1.0f, 0.0f }));
            skr::math::float3 camera_up = skr::math::cross(right, forward);

            // 移动速度基于帧时间
            float speed = move_speed * delta_time;

            // WASD移动
            if (keys_pressed['w'] || keys_pressed['W']) position += forward * speed;
            if (keys_pressed['s'] || keys_pressed['S']) position -= forward * speed;
            if (keys_pressed['a'] || keys_pressed['A']) position -= right * speed;
            if (keys_pressed['d'] || keys_pressed['D']) position += right * speed;
            if (keys_pressed['q'] || keys_pressed['Q']) position.y -= speed; // 下降
            if (keys_pressed['e'] || keys_pressed['E']) position.y += speed; // 上升

            // 更新target为position + forward
            target = position + forward;
            up = camera_up;
        }

        void initialize_from_lookat(const skr::math::float3& eye, const skr::math::float3& look_target)
        {
            position = eye;
            target = look_target;

            // 计算初始的yaw和pitch
            skr::math::float3 direction = skr::math::normalize(look_target - eye);
            yaw = atan2(direction.z, direction.x);
            pitch = asin(direction.y);
        }
    } camera_controller;
};

IMPLEMENT_DYNAMIC_MODULE(RGRaytracingSampleModule, RenderGraphRaytracingSample);

// Helper function to read shader bytes
inline static void read_shader_bytes(const char* virtual_path, uint32_t** bytes, uint32_t* length, ECGPUBackend backend)
{
    char shader_file[256];
    const char* shader_path = "./../resources/shaders/";
    strcpy(shader_file, shader_path);
    strcat(shader_file, virtual_path);
    switch (backend)
    {
    case CGPU_BACKEND_VULKAN:
        strcat(shader_file, ".spv");
        break;
    case CGPU_BACKEND_D3D12:
    case CGPU_BACKEND_XBOX_D3D12:
        strcat(shader_file, ".dxil");
        break;
    case CGPU_BACKEND_METAL:
        strcat(shader_file, ".metallib");
        break;
    default:
        SKR_UNIMPLEMENTED_FUNCTION();
        break;
    }

    FILE* f = fopen(shader_file, "rb");
    if (!f)
    {
        SKR_LOG_ERROR(u8"Failed to open shader file: %s", shader_file);
        return;
    }
    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);
    *bytes = (uint32_t*)malloc(*length + 1);
    fread(*bytes, *length, 1, f);
    fclose(f);
}

RGRaytracingSampleModule* RGRaytracingSampleModule::Get()
{
    auto mm = skr_get_module_manager();
    static auto rm = static_cast<RGRaytracingSampleModule*>(mm->get_module(u8"RenderGraphRaytracingSample"));
    return rm;
}

void RGRaytracingSampleModule::on_load(int argc, char8_t** argv)
{
    skr_thread_sleep(1000);

    scheduler.initialize({});
    scheduler.bind();
    world.initialize();
    transform_system = skr::TransformSystem::Create(&world);

    spawn_entities();
    create_as();

    render_graph = skr::RG::RenderGraph::create(
        [=, this](skr::RG::RenderGraphBuilder& builder) {
            builder.with_device(device)
                .with_gfx_queue(gfx_queue);
        }
    );
}

int RGRaytracingSampleModule::main_module_exec(int argc, char8_t** argv)
{
    app.initialize(nullptr);
    auto wm = app.get_window_manager();
    auto eq = app.get_event_queue();
    auto main_window = wm->create_window({ .title = u8"Render Graph Raytracing Sample", .size = { 1280, 720 }, .is_resizable = false });
    main_window->show();
    SKR_DEFER({ wm->destroy_window(main_window); });

    // Create swapchain for profiler hook support
    create_swapchain(main_window);

    static bool want_quit = false;
    struct eventListener : public skr::ISystemEventHandler
    {
        RGRaytracingSampleModule* module = nullptr;

        void handle_event(const SkrSystemEvent& event) SKR_NOEXCEPT
        {
            if (event.type == SKR_SYSTEM_EVENT_QUIT)
                want_quit = true;
            if (event.type == SKR_SYSTEM_EVENT_WINDOW_CLOSE_REQUESTED)
                want_quit = event.window.window_native_handle == main_window->get_native_handle();

            // 处理键盘输入
            if (event.type == SKR_SYSTEM_EVENT_KEY_DOWN)
            {
                module->camera_controller.keys_pressed[event.key.keycode] = true;
            }
            else if (event.type == SKR_SYSTEM_EVENT_KEY_UP)
            {
                module->camera_controller.keys_pressed[event.key.keycode] = false;
            }

            // 处理鼠标输入
            if (event.type == SKR_SYSTEM_EVENT_MOUSE_BUTTON_DOWN)
            {
                if (event.mouse.button == InputMouseButtonFlags::InputMouseRightButton)
                {
                    module->camera_controller.right_mouse_pressed = true;
                    module->camera_controller.last_mouse_x = event.mouse.x;
                    module->camera_controller.last_mouse_y = event.mouse.y;
                }
            }
            else if (event.type == SKR_SYSTEM_EVENT_MOUSE_BUTTON_UP)
            {
                if (event.mouse.button == InputMouseButtonFlags::InputMouseRightButton)
                {
                    module->camera_controller.right_mouse_pressed = false;
                }
            }
            else if (event.type == SKR_SYSTEM_EVENT_MOUSE_MOVE)
            {
                if (module->camera_controller.right_mouse_pressed)
                {
                    float dx = event.mouse.x - module->camera_controller.last_mouse_x;
                    float dy = event.mouse.y - module->camera_controller.last_mouse_y;

                    module->camera_controller.yaw += dx * module->camera_controller.mouse_sensitivity;
                    module->camera_controller.pitch -= dy * module->camera_controller.mouse_sensitivity; // 反向Y轴

                    // 限制pitch角度
                    module->camera_controller.pitch = std::max(-1.5f, std::min(1.5f, module->camera_controller.pitch));

                    module->camera_controller.last_mouse_x = event.mouse.x;
                    module->camera_controller.last_mouse_y = event.mouse.y;
                }
            }
        }
        skr::SystemWindow* main_window = nullptr;
    } eventListener;
    eventListener.main_window = main_window;
    eventListener.module = this;
    eq->add_handler(&eventListener);
    SKR_DEFER({ eq->remove_handler(&eventListener); });

    // 初始化相机控制器
    this->camera_controller.initialize_from_lookat(
        { -25000.0f, 15000.0f, -35000.0f },
        { 0.0f, 2000.0f, 0.0f }
    );

    // 时间管理
    auto last_time = std::chrono::high_resolution_clock::now();

    while (!want_quit)
    {
        eq->pump_messages();

        // transform_system->update();

        // 计算帧时间
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;

        // 更新相机
        this->camera_controller.update_camera(delta_time);

        render();

        {
            SkrZoneScopedN("Sync");
            skr::ecs::TaskScheduler::Get()->sync_all();
        }
    }

    return 0;
}

// Three attachment levels hierarchy
static constexpr int LEVEL_1_COUNT = 100;   // Root level: 100 entities (cities)
static constexpr int LEVEL_2_COUNT = 900;   // Mid level: 900 entities (buildings) - 9 per city
static constexpr int LEVEL_3_COUNT = 45000; // Leaf level: 45000 entities (objects) - ~50 per building
void RGRaytracingSampleModule::spawn_entities()
{
    using namespace skr::ecs;

    // Scene dimensions: 2000x2000x2000 units
    constexpr float SCENE_SIZE = 2000.0f;
    constexpr int TOTAL_ENTITIES = 50000;

    static std::atomic_uint32_t entities_count = 0;
    struct LevelSpawner
    {
        void build(skr::ecs::ArchetypeBuilder& Builder)
        {
            Builder.add_component<skr::SolvedTransformComponent>()
                .add_component(&LevelSpawner::children)
                .add_component(&LevelSpawner::translations)
                .add_component(&LevelSpawner::rotations)
                .add_component(&LevelSpawner::scales);
        }

        skr::Vector<Entity> ents;

        ComponentView<skr::ChildrenComponent> children;
        ComponentView<skr::PositionComponent> translations;
        ComponentView<skr::RotationComponent> rotations;
        ComponentView<skr::ScaleComponent> scales;
    };

    // Level 1 spawner (Cities) - Root entities without parents
    struct Level1Spawner : public LevelSpawner
    {
        void run(skr::ecs::TaskContext& Context)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<skr::real> pos_dist(-SCENE_SIZE * 0.5f, SCENE_SIZE * 0.5f);
            std::uniform_real_distribution<float> scale_dist(0.5f, 3.0f);
            std::uniform_real_distribution<float> rotation_dist(0.0f, 2.0f * skr::kPi);

            ents.append(Context.entities(), Context.size());
            for (int i = 0; i < Context.size(); ++i)
            {
                // Position cities in a grid with some randomness
                int grid_x = i % 10;
                int grid_z = i / 10;
                float base_x = (grid_x - 4.5) * (SCENE_SIZE * 0.9 / 10.0);
                float base_z = (grid_z - 4.5) * (SCENE_SIZE * 0.9 / 10.0);
                float city_scale = scale_dist(gen) * 4.0f; // Moderate scale

                translations[i].set(
                    { base_x + pos_dist(gen) * 0.1f,
                      0.0f,
                      base_z + pos_dist(gen) * 0.1f }
                );
                rotations[i].set({ 0.0f, rotation_dist(gen), 0.0f });
                scales[i].set({ city_scale, city_scale, city_scale });
            }
        }
    } level1_spawner;

    // Level 2 spawner (Buildings) - With parent relationships
    struct Level2Spawner : public LevelSpawner
    {
        const Level1Spawner& lv1;
        Level2Spawner(const Level1Spawner& lv1)
            : lv1(lv1)
        {
        }

        void build(skr::ecs::ArchetypeBuilder& Builder)
        {
            LevelSpawner::build(Builder);
            Builder.add_component(&Level2Spawner::parents)
                .access(&Level2Spawner::children_writer);
        }

        void run(skr::ecs::TaskContext& Context)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<skr::real> pos_dist(-SCENE_SIZE * 0.5f, SCENE_SIZE * 0.5f);
            std::uniform_real_distribution<float> scale_dist(0.5f, 3.0f);
            std::uniform_real_distribution<float> rotation_dist(0.0f, 2.0f * skr::kPi);

            ents.append(Context.entities(), Context.size());
            for (int i = 0; i < Context.size(); ++i)
            {
                // Position buildings around city centers
                float angle = (i % 9) * (2.0f * skr::kPi / 9.0f) + rotation_dist(gen) * 0.1f;
                float radius = 100.0f + pos_dist(gen) * 0.1f;
                float building_scale = scale_dist(gen) * 2.0f; // Medium scale for buildings

                translations[i].set(
                    { std::cos(angle) * radius,
                      pos_dist(gen) * 10.0f,
                      std::sin(angle) * radius }
                );
                rotations[i].set({ 0.0f, rotation_dist(gen), 0.0f });
                scales[i].set({ building_scale, building_scale, building_scale });

                const auto parent = lv1.ents[index_in_level / 9];
                skr::ChildrenComponent as_child = { .entity = Context.entities()[i] };
                parents[i].entity = parent;                  // Attach to the corresponding city
                children_writer[parent].push_back(as_child); // Add this building to the city's children
                index_in_level += 1;
            }
        }
        uint64_t index_in_level = 0;
        ComponentView<skr::ParentComponent> parents;
        RandomComponentReadWrite<skr::ChildrenComponent> children_writer;
    } level2_spawner(level1_spawner);

    // Level 3 spawner (Objects) - Leaf entities
    struct Level3Spawner : public LevelSpawner
    {
        const Level2Spawner& lv2;
        Level3Spawner(const Level2Spawner& lv2)
            : lv2(lv2)
        {
        }

        void build(skr::ecs::ArchetypeBuilder& Builder)
        {
            LevelSpawner::build(Builder);
            Builder.add_component(&Level3Spawner::parents)
                .access(&Level2Spawner::children_writer);
        }

        void run(skr::ecs::TaskContext& Context)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<skr::real> pos_dist(-SCENE_SIZE * 0.5f, SCENE_SIZE * 0.5f);
            std::uniform_real_distribution<float> scale_dist(0.5f, 3.0f);
            std::uniform_real_distribution<float> rotation_dist(0.0f, 2.0f * skr::kPi);

            ents.append(Context.entities(), Context.size());
            for (int i = 0; i < Context.size(); ++i)
            {
                // Distribute objects around buildings
                float local_radius = 50.0f;
                float local_angle = rotation_dist(gen);
                float local_distance = pos_dist(gen) * 0.01f * local_radius;

                translations[i].set(
                    { std::cos(local_angle) * local_distance,
                      pos_dist(gen) * 5.0f,
                      std::sin(local_angle) * local_distance }
                );

                rotations[i].set(
                    { rotation_dist(gen) * 0.2f,  // Small pitch variation
                      rotation_dist(gen),         // Full yaw rotation
                      rotation_dist(gen) * 0.1f } // Small roll variation
                );

                float object_scale = scale_dist(gen) * 1.0f; // Small scale for objects
                scales[i].set({ object_scale, object_scale, object_scale });

                const auto parent = lv2.ents[index_in_level / 50];
                skr::ChildrenComponent as_child = { .entity = Context.entities()[i] };
                parents[i].entity = parent;                  // Attach to the corresponding building
                children_writer[parent].push_back(as_child); // Add this object to the building's children
                index_in_level += 1;
            }
        }
        uint64_t index_in_level = 0;
        ComponentView<skr::ParentComponent> parents;
        RandomComponentReadWrite<skr::ChildrenComponent> children_writer;
    } level3_spawner(level2_spawner);

    {
        SkrZoneScopedN("SpawnLevel1");
        world.create_entities(level1_spawner, LEVEL_1_COUNT);
    }
    {
        SkrZoneScopedN("SpawnLevel2");
        world.create_entities(level2_spawner, LEVEL_2_COUNT);
    }
    {
        SkrZoneScopedN("SpawnLevel3");
        world.create_entities(level3_spawner, LEVEL_3_COUNT);
    }

    {
        SkrZoneScopedN("UpdateTransformSystem");
        transform_system->update();
    }

    SKR_LOG_INFO(u8"Spawned hierarchical scene with {%d} entities:", LEVEL_1_COUNT + LEVEL_2_COUNT + LEVEL_3_COUNT);
    SKR_LOG_INFO(u8"  Level 1 (Cities): {%d} entities", LEVEL_1_COUNT);
    SKR_LOG_INFO(u8"  Level 2 (Buildings): {%d} entities", LEVEL_2_COUNT);
    SKR_LOG_INFO(u8"  Level 3 (Objects): {%d} entities", LEVEL_3_COUNT);
    SKR_LOG_INFO(u8"Scene distributed in {%d}x{%d}x{%d} space", static_cast<int>(SCENE_SIZE), static_cast<int>(SCENE_SIZE), static_cast<int>(SCENE_SIZE));
    SKR_LOG_INFO(u8"Established parent-child relationships successfully");
}

void RGRaytracingSampleModule::create_as()
{
    // Initialize CGPU
    CGPUInstanceDescriptor instance_desc = {
#if SKR_PLAT_WINDOWS
        .backend = CGPU_BACKEND_D3D12,
#elif SKR_PLAT_MACOSX
        .backend = CGPU_BACKEND_METAL,
#endif
        .enable_debug_layer = false,
        .enable_gpu_based_validation = false,
        .enable_set_name = true
    };
    instance = cgpu_create_instance(&instance_desc);

    // Get adapter and create device
    uint32_t adapters_count = 0;
    cgpu_enum_adapters(instance, nullptr, &adapters_count);
    skr::Vector<CGPUAdapterId> adapters;
    adapters.resize_zeroed(adapters_count);
    cgpu_enum_adapters(instance, adapters.data(), &adapters_count);
    CGPUAdapterId adapter = adapters[0];

    CGPUQueueGroupDescriptor queue_group = {
        .queue_type = CGPU_QUEUE_TYPE_GRAPHICS,
        .queue_count = 1
    };
    CGPUDeviceDescriptor device_desc = {
        .queue_groups = &queue_group,
        .queue_group_count = 1
    };
    device = cgpu_create_device(adapter, &device_desc);
    gfx_queue = cgpu_get_queue(device, CGPU_QUEUE_TYPE_GRAPHICS, 0);

    // Create sphere geometry (icosphere or simple sphere)
    create_sphere_blas();

    // Create TLAS from ECS entities
    create_SceneTLAS();

    // Create compute pipeline and resources
    create_compute_pipeline();

    SKR_LOG_INFO(u8"Created raytracing acceleration structures:");
    SKR_LOG_INFO(u8"  BLAS: Sphere geometry");
}

void RGRaytracingSampleModule::create_sphere_blas()
{
    SkrZoneScopedN("CreateSphereBLAS");

    // Generate sphere vertices and indices (simple icosphere or UV sphere)
    constexpr float radius = 3.0f;
    constexpr int segments = 16;
    constexpr int rings = 8;

    skr::Vector<skr::float3> vertices;
    skr::Vector<uint16_t> indices;

    // Generate sphere vertices (UV sphere)
    for (int ring = 0; ring <= rings; ++ring)
    {
        float phi = skr::kPi * ring / rings;
        float sin_phi = std::sin(phi);
        float cos_phi = std::cos(phi);

        for (int segment = 0; segment <= segments; ++segment)
        {
            float theta = 2.0f * skr::kPi * segment / segments;
            float sin_theta = std::sin(theta);
            float cos_theta = std::cos(theta);

            skr::float3 vertex = {
                radius * sin_phi * cos_theta,
                radius * cos_phi,
                radius * sin_phi * sin_theta
            };
            vertices.add(vertex);
        }
    }

    // Generate sphere indices
    for (int ring = 0; ring < rings; ++ring)
    {
        for (int segment = 0; segment < segments; ++segment)
        {
            int current = ring * (segments + 1) + segment;
            int next = current + segments + 1;

            // First triangle
            indices.add(current);
            indices.add(next);
            indices.add(current + 1);

            // Second triangle
            indices.add(current + 1);
            indices.add(next);
            indices.add(next + 1);
        }
    }

    // Create vertex buffer
    CGPUBufferDescriptor vertex_buffer_desc = {
        .size = vertices.size() * sizeof(skr::float3),
        .name = u8"SphereVertexBuffer",
        .usages = CGPU_BUFFER_USAGE_VERTEX_BUFFER,
        .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
        .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT,
        .start_state = CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
    };
    sphere_vertex_buffer = cgpu_create_buffer(device, &vertex_buffer_desc);
    memcpy(sphere_vertex_buffer->info->cpu_mapped_address, vertices.data(), vertex_buffer_desc.size);

    // Create index buffer
    CGPUBufferDescriptor index_buffer_desc = {
        .size = indices.size() * sizeof(uint16_t),
        .name = u8"SphereIndexBuffer",
        .usages = CGPU_BUFFER_USAGE_INDEX_BUFFER,
        .memory_usage = CGPU_MEM_USAGE_CPU_TO_GPU,
        .flags = CGPU_BUFFER_FLAG_PERSISTENT_MAP_BIT,
        .start_state = CGPU_RESOURCE_STATE_INDEX_BUFFER
    };
    sphere_index_buffer = cgpu_create_buffer(device, &index_buffer_desc);
    memcpy(sphere_index_buffer->info->cpu_mapped_address, indices.data(), index_buffer_desc.size);

    // Create BLAS
    SKR_DECLARE_ZERO(CGPUAccelerationStructureGeometryDesc, blas_geom);
    blas_geom.flags = CGPU_ACCELERATION_STRUCTURE_GEOMETRY_FLAG_OPAQUE;
    blas_geom.vertex_buffer = sphere_vertex_buffer;
    blas_geom.index_buffer = sphere_index_buffer;
    blas_geom.vertex_count = vertices.size();
    blas_geom.vertex_stride = sizeof(skr::float3);
    blas_geom.vertex_format = CGPU_FORMAT_R32G32B32_SFLOAT;
    blas_geom.index_count = indices.size();
    blas_geom.index_stride = sizeof(uint16_t);
    blas_geom.transform[0] = 1.f;
    blas_geom.transform[5] = 1.f;
    blas_geom.transform[10] = 1.f;

    SKR_DECLARE_ZERO(CGPUAccelerationStructureDescriptor, blas_desc);
    blas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    blas_desc.flags = CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    blas_desc.bottom.count = 1;
    blas_desc.bottom.geometries = &blas_geom;
    sphere_blas = cgpu_create_acceleration_structure(device, &blas_desc);
}

void RGRaytracingSampleModule::create_SceneTLAS()
{
    using namespace skr::ecs;

    // Collect all entities with transform components
    skr::Vector<CGPUAccelerationStructureInstanceDesc> tlas_instances;
    {
        SkrZoneScopedN("ReserveUpdateBuffer");
        tlas_instances.resize_default(LEVEL_1_COUNT + LEVEL_2_COUNT + LEVEL_3_COUNT);
    }
    // Query all entities with translation, rotation, and scale components using a system job
    struct TransformJob
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.read(&TransformJob::transforms);
        }
        ComponentView<const skr::SolvedTransformComponent> transforms;
    };
    struct GatherTransforms : public TransformJob
    {
        CGPUAccelerationStructureId sphere_blas = nullptr;
        skr::Vector<CGPUAccelerationStructureInstanceDesc>* pInstances = nullptr;
        std::atomic_uint32_t* pInstanceCounter = nullptr;

        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("GatherTransforms");
            for (int i = 0; i < Context.size(); ++i)
            {
                // Create transform matrix from translation, rotation, scale
                const auto transform = transforms[i].get().to_matrix();
                auto instance_id = pInstanceCounter->fetch_add(1);

                auto& instance = (*pInstances)[instance_id];
                instance.bottom = sphere_blas;
                instance.instance_id = instance_id;
                instance.instance_mask = 255;

                // Use computed transform matrix (now that we know the format is correct)
                float transform34[12] = {
                    transform.m00, transform.m10, transform.m20, transform.m30, // Row 0: X axis + X translation
                    transform.m01,
                    transform.m11,
                    transform.m21,
                    transform.m31, // Row 1: Y axis + Y translation
                    transform.m02,
                    transform.m12,
                    transform.m22,
                    transform.m32 // Row 2: Z axis + Z translation
                };
                memcpy(instance.transform, transform34, sizeof(transform34));
            }
        }
    } transform_job;
    std::atomic_uint32_t instanceCounter;
    transform_job.sphere_blas = sphere_blas;
    transform_job.pInstances = &tlas_instances;
    transform_job.pInstanceCounter = &instanceCounter;

    // Execute the job to collect transform data
    {
        SkrZoneScopedN("QueryTransformsOut");
        auto query1 = world.dispatch_task(transform_job, UINT32_MAX, nullptr);
        TaskScheduler::Get()->sync_all();
        world.destroy_query(query1);
    }

    // Create TLAS
    {
        SkrZoneScopedN("CreateSceneTLAS");
        CGPUAccelerationStructureDescriptor tlas_desc = {};
        tlas_desc.type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        tlas_desc.flags = CGPU_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
        tlas_desc.top.count = tlas_instances.size();
        tlas_desc.top.instances = tlas_instances.data();
        SceneTLAS = cgpu_create_acceleration_structure(device, &tlas_desc);
    }
    {
        SkrZoneScopedN("BuildSceneTLAS");
        // Build acceleration structures
        CGPUCommandPoolId pool = cgpu_create_command_pool(gfx_queue, nullptr);
        CGPUCommandBufferDescriptor cmd_desc = { .is_secondary = false };
        CGPUCommandBufferId cmd = cgpu_create_command_buffer(pool, &cmd_desc);

        cgpu_cmd_begin(cmd);

        // Build BLAS first
        CGPUAccelerationStructureBuildDescriptor blas_build = {
            .type = CGPU_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
            .as_count = 1,
            .as = &sphere_blas
        };
        cgpu_cmd_build_acceleration_structures(cmd, &blas_build);

        // Build TLAS
        CGPUAccelerationStructureBuildDescriptor tlas_build = {
            .type = CGPU_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
            .as_count = 1,
            .as = &SceneTLAS
        };
        cgpu_cmd_build_acceleration_structures(cmd, &tlas_build);

        cgpu_cmd_end(cmd);

        CGPUQueueSubmitDescriptor submit_desc = {
            .cmds = &cmd,
            .cmds_count = 1
        };
        cgpu_submit_queue(gfx_queue, &submit_desc);
        cgpu_wait_queue_idle(gfx_queue);

        cgpu_free_command_buffer(cmd);
        cgpu_free_command_pool(pool);
    }
}

void RGRaytracingSampleModule::create_compute_pipeline()
{
    // Create compute shader
    uint32_t *shader_bytes, shader_length;
    read_shader_bytes("rg-raytracing/rg-raytracing.cs_main", &shader_bytes, &shader_length, device->adapter->instance->backend);
    CGPUShaderLibraryDescriptor shader_desc = {
        .name = u8"RayTracingComputeShader",
        .code = shader_bytes,
        .code_size = shader_length
    };
    compute_shader = cgpu_create_shader_library(device, &shader_desc);
    free(shader_bytes);

    // Create root signature
    const char8_t* push_constant_name = SKR_UTF8("camera_constants");
    CGPUShaderEntryDescriptor compute_shader_entry = {
        .library = compute_shader,
        .entry = u8"cs_main"
    };
    CGPURootSignatureDescriptor root_desc = {
        .shaders = &compute_shader_entry,
        .shader_count = 1,
        .push_constant_names = &push_constant_name,
        .push_constant_count = 1,
    };
    root_signature = cgpu_create_root_signature(device, &root_desc);

    // Create compute pipeline
    CGPUComputePipelineDescriptor pipeline_desc = {
        .root_signature = root_signature,
        .compute_shader = &compute_shader_entry,
    };
    compute_pipeline = cgpu_create_compute_pipeline(device, &pipeline_desc);
}

void RGRaytracingSampleModule::create_swapchain(skr::SystemWindow* window)
{
    if (!window || !device) return;

    CGPUSurfaceId surface = cgpu_surface_from_native_view(device, window->get_native_view());

    CGPUSwapChainDescriptor swapchain_desc = {
        .present_queues = &gfx_queue,
        .present_queues_count = 1,
        .surface = surface,
        .image_count = 2,
        .width = 1280,
        .height = 720,
        .enable_vsync = false,
        .format = CGPU_FORMAT_R8G8B8A8_UNORM,
    };
    swapchain = cgpu_create_swapchain(device, &swapchain_desc);
}

void RGRaytracingSampleModule::render()
{
    using namespace skr;

    // Camera constants structure matching shader
    struct CameraConstants
    {
        skr::math::float4 cameraPos;
        skr::math::float4 cameraDir;
        skr::math::float2 screenSize;
    };

    if (!swapchain || !render_graph)
        return;

    SkrZoneScopedN("Render");

    // Acquire next image
    CGPUAcquireNextDescriptor acquire_desc = {};
    uint8_t backbuffer_index = 0;
    {
        SkrZoneScopedN("AcquireBackBuffer");
        backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire_desc);
    }
    CGPUTextureId to_import = swapchain->back_buffers[backbuffer_index];

    // Setup camera (looking at the scene from a distance)
    CameraConstants camera_constants = {};

    // Camera setup
    const float nearPlane = 0.1f;
    const float farPlane = 99999999.0f;

    // Use camera controller values
    skr::math::float3 eye = this->camera_controller.position;
    skr::math::float3 target = this->camera_controller.target;
    skr::math::float3 up = this->camera_controller.up;

    camera_constants.cameraPos = skr::math::float4(eye, 0.f);
    camera_constants.cameraDir = skr::math::float4(skr::math::normalize(target - eye), 0.f);
    camera_constants.screenSize = { static_cast<float>(to_import->info->width),
                                    static_cast<float>(to_import->info->height) };

    // Create intermediate render target texture (can create UAV)
    auto render_target_handle = render_graph->create_texture(
        [=](RG::RenderGraph& g, RG::TextureBuilder& builder) {
            builder.set_name(u8"raytracing_output")
                .extent(to_import->info->width, to_import->info->height, 1)
                .format(to_import->info->format)
                .allow_readwrite(); // Enable UAV creation
        }
    );

    // Create backbuffer texture handle for RenderGraph (import only, no UAV)
    auto backbuffer_handle = render_graph->create_texture(
        [=](RG::RenderGraph& g, RG::TextureBuilder& builder) {
            builder.set_name(u8"backbuffer")
                .import(to_import, CGPU_RESOURCE_STATE_PRESENT);
        }
    );

    // Create acceleration structure handle for RenderGraph
    auto tlas_handle = render_graph->create_acceleration_structure(
        [=, this](RG::RenderGraph& g, RG::AccelerationStructureBuilder& builder) {
            builder.set_name(u8"SceneTLAS")
                .import(SceneTLAS);
        }
    );

    // Add raytracing compute pass (write to intermediate texture)
    render_graph->add_compute_pass(
        [=, this](RG::RenderGraph& g, RG::ComputePassBuilder& builder) {
            builder.set_name(u8"RayTracingPass")
                .set_pipeline(compute_pipeline)
                .read(u8"SceneTLAS", tlas_handle)
                .readwrite(u8"output_texture", render_target_handle);
        },
        [=, this](RG::RenderGraph& g, RG::ComputePassContext& ctx) {
            // Push constants
            cgpu_compute_encoder_push_constants(ctx.encoder, root_signature, u8"camera_constants", &camera_constants);

            // Dispatch compute shader
            cgpu_compute_encoder_set_threadgroup_size(ctx.encoder, 16, 16, 1);
            cgpu_compute_encoder_dispatch(ctx.encoder, camera_constants.screenSize.x, camera_constants.screenSize.y, 1);
        }
    );

    // Add copy pass to copy intermediate texture to backbuffer
    render_graph->add_copy_pass(
        [=, this](RG::RenderGraph& g, RG::CopyPassBuilder& builder) {
            builder.set_name(u8"CopyToBackbuffer")
                .texture_to_texture(
                    render_target_handle,
                    backbuffer_handle,
                    CGPU_RESOURCE_STATE_PRESENT
                );
        },
        [=, this](RG::RenderGraph& g, RG::CopyPassContext& ctx) {
            // Copy pass execution is handled automatically by RenderGraph
        }
    );

    // Add present pass
    render_graph->add_present_pass(
        [=, this](RG::RenderGraph& g, RG::PresentPassBuilder& builder) {
            builder.set_name(u8"present")
                .swapchain(swapchain, backbuffer_index)
                .texture(backbuffer_handle, true);
        }
    );

    // Execute render graph
    {
        SkrZoneScopedN("ExecuteRenderGraph");
        render_graph->execute();
    }

    // Present
    CGPUQueuePresentDescriptor present_desc = {
        .swapchain = swapchain,
        .index = backbuffer_index
    };
    cgpu_queue_present(gfx_queue, &present_desc);
}

void RGRaytracingSampleModule::on_unload()
{
    cgpu_wait_queue_idle(gfx_queue);

    // Clean up render graph
    if (render_graph)
    {
        skr::RG::RenderGraph::destroy(render_graph);
        render_graph = nullptr;
    }

    // Clean up compute pipeline resources
    if (compute_pipeline) cgpu_free_compute_pipeline(compute_pipeline);
    if (root_signature) cgpu_free_root_signature(root_signature);
    if (compute_shader) cgpu_free_shader_library(compute_shader);

    // Clean up swapchain resources
    if (swapchain) cgpu_free_swapchain(swapchain);

    // Clean up raytracing resources
    if (SceneTLAS) cgpu_free_acceleration_structure(SceneTLAS);
    if (sphere_blas) cgpu_free_acceleration_structure(sphere_blas);
    if (sphere_vertex_buffer) cgpu_free_buffer(sphere_vertex_buffer);
    if (sphere_index_buffer) cgpu_free_buffer(sphere_index_buffer);
    if (gfx_queue) cgpu_free_queue(gfx_queue);
    if (device) cgpu_free_device(device);
    if (instance) cgpu_free_instance(instance);

    transform_system.reset();
    world.finalize();
    app.shutdown();
    scheduler.unbind();
}