#pragma once
#include "SkrBase/config.h"
#include <SkrCore/memory/sp.hpp>

namespace skr::ecs
{
struct ECSWorld;
}

namespace skr
{
struct SKR_SCENE_API TransformSystem
{
public:
    static UPtr<TransformSystem> Create(skr::ecs::ECSWorld* world);
    static void Destroy(TransformSystem* system);
    void update();
    void wait();

private:
    // 正向全量脏树传播：每帧调用，处理所有标记为脏的变换树，进行批量计算
    void CalculateFromRoot();

private:
    TransformSystem() = default;
    ~TransformSystem() = default;
    struct Impl;
    Impl* impl;
};

template <>
struct SPDeleterTraits<TransformSystem>
{
    inline static void do_delete(TransformSystem* p)
    {
        TransformSystem::Destroy(p);
    }
};

} // namespace skr