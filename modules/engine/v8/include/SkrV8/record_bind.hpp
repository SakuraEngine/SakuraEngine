#pragma once
#include "SkrContainers/vector.hpp"
#include "SkrGuid/guid.hpp"

namespace skr::v8_bind
{
struct ClassBindInfo;
struct MethodBindInfo;
struct FieldInfo;
struct StaticMethodInfo;
struct StaticFieldInfo;

struct ClassBindInfo {
    // class basic
    GUID type_id;
    GUID super_id;

    // method & fields
    Vector<MethodBindInfo> methods;
    Vector<FieldInfo>      fields;

    // static fields & static methods
    Vector<StaticMethodInfo> static_methods;
    Vector<StaticFieldInfo>  static_fields;
};

template <typename T>
struct Class_ {
    template <typename Ctor>
    void ctor();

private:
    ClassBindInfo _bind_info;
};

} // namespace skr::v8_bind