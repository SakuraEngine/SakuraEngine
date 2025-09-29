#pragma once
#include "SkrRuntime/ecs/component.hpp"
#include "SkrBase/math.hpp"
#include "SkrScene/basic_components.generated.h" // IWYU pragma: export
#include <SkrRuntime/sugoi/sugoi_meta.hpp>

// transform components
namespace skr
{
struct [[secs_component, sattr(
    guid = "01981176-5d6c-777a-8b33-feb16c1de2b6";
    serde = @enable;
    ecs.comp = @enable;
)]] RotationComponent
{
public:
    SKR_GENERATE_BODY(RotationComponent)

    // ctor & dtor
    inline RotationComponent() = default;
    inline RotationComponent(Rotator v)
        : _rotator(v)
        , _dirty(true)
    {
    }

    // getter & setter
    inline void set(Rotator v)
    {
        _rotator = v;
        _dirty = true;
    }
    inline auto get() const { return _rotator; }

    // dirty control
    inline bool is_dirty() const { return _dirty; }
    inline void mark_dirty() const { _dirty = true; }
    inline void cancel_dirty() const { _dirty = false; }

private:
    Rotator _rotator = {};

    [[sattr(serde = @disable)]]
    mutable bool _dirty = false;
};

struct [[secs_component, sattr(
    guid = "01981176-7361-72ca-a0c5-5b539d1764e1";
    serde = @enable
    ecs.comp = @enable;
)]] SKR_ALIGNAS(16) PositionComponent
{
public:
    SKR_GENERATE_BODY(PositionComponent)

    // ctor & dtor
    inline PositionComponent() = default;
    inline PositionComponent(Position v)
        : _position(v)
        , _dirty(true)
    {
    }

    // getter & setter
    inline void set(Position v)
    {
        _position = v;
        _dirty = true;
    }
    inline Position get() const { return _position; }

    // dirty control
    inline bool is_dirty() const { return _dirty; }
    inline void mark_dirty() const { _dirty = true; }
    inline void cancel_dirty() const { _dirty = false; }

private:
    Position _position = {};

    [[sattr(serde = @disable)]]
    mutable bool _dirty = false;
};

struct [[secs_component, sattr(
    guid = "01981176-8537-72d1-88a4-e459116c0717";
    serde = @enable
    ecs.comp = @enable;)]]
SKR_ALIGNAS(16) ScaleComponent
{
public:
    SKR_GENERATE_BODY(ScaleComponent)

    // ctor & dtor
    inline ScaleComponent() = default;
    inline ScaleComponent(Scale scale)
        : _scale(scale)
        , _dirty(true)
    {
    }

    // getter & setter
    inline void set(Scale scale)
    {
        _scale = scale;
        _dirty = true;
    }
    inline Scale get() const { return _scale; }

    // dirty control
    inline bool is_dirty() const { return _dirty; }
    inline void mark_dirty() const { _dirty = true; }
    inline void cancel_dirty() const { _dirty = false; }

private:
    Scale _scale = { 1, 1, 1 };

    [[sattr(serde = @disable)]]
    mutable bool _dirty;
};

struct [[secs_component, sattr(
    guid = "01981176-4865-779f-9e5d-ebaf834fc004";
    serde = @enable
    ecs.comp = @enable;
)]] SKR_ALIGNAS(16) SolvedTransformComponent
{
public:
    SKR_GENERATE_BODY(SolvedTransformComponent)

    // ctor & dtor
    inline SolvedTransformComponent(Transform transform = Transform::Identity())
        : _transform(transform)
    {
    }

    // getter & setter
    inline Transform get() const { return _transform; }
    inline void set(Transform transform) { _transform = transform; }

private:
    Transform _transform;
};

struct [[secs_component, sattr(
    guid = "01981177-1059-7687-8b72-50788033fdd4";
    ecs.comp = @enable;
)]] ParentComponent
{
    skr::ecs::Entity entity;
};

struct [[secs_component, sattr(
    guid = "01981176-f870-738b-8cac-5e3d87ba37b5";
    ecs.comp.array = 4;
)]] ChildrenComponent
{
    skr::ecs::Entity entity;
};

using ChildrenArray = sugoi::ArrayComponent<ChildrenComponent, 4>;
} // namespace skr