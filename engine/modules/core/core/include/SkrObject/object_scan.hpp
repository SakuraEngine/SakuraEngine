#pragma once
#include <SkrObject/fwd.hpp>
#include <SkrObject/object.hpp>
#include <SkrObject/object_ptr.hpp>

namespace skr
{
template <typename T>
struct ObjectScan
{
};

// for directly object use
template <concepts::BasedOnObject T>
struct ObjectScan<T>
{
    static_assert(std::is_same_v<T, T*>, "cannot use Object as value, use ObjPtr<T> instead");
};

// for object pointers
template <concepts::BasedOnObject T>
struct ObjectScan<T*>
{
    static_assert(std::is_same_v<T, T*>, "cannot use Object* directly, use ObjPtr<T> instead");
};
template <concepts::BasedOnObject T>
struct ObjectScan<const T*> : ObjectScan<T*>
{
};
template <concepts::BasedOnObject T>
struct ObjectScan<volatile T*> : ObjectScan<T*>
{
};

// for object ptrs
// TODO. 直接取得内部的 ObjPtrBasic 来扫描
template <concepts::BasedOnObject T>
struct ObjectScan<ObjPtr<const T>>
{
    inline static void gc(ObjectScanCallbackGC callback, const ObjPtr<const T>& ptr)
    {
    }
    inline static void package(ObjectScanCallbackPackage callback, const ObjPtr<const T>& ptr)
    {
    }
};
template <concepts::BasedOnObject T>
struct ObjectScan<ObjPtr<T>>
{
    inline static void gc(ObjectScanCallbackGC callback, const ObjPtr<T>& ptr)
    {
    }
    inline static void package(ObjectScanCallbackPackage callback, const ObjPtr<T>& ptr)
    {
    }
};
} // namespace skr