#pragma once
#include <SkrObject/fwd.hpp>
#include <SkrCore/memory/rc.hpp>
#include <SkrBase/types.h>

// TODO. ObjPtr 的 Resolve 在 Package 阶段就应当完全完成，因此不存在 ObjectResolver
// TODO. ObjPtrSoft 应当存在 ObjectResolver 用于 LazyResolve，且其 Resolve 与 Package 完全无关，
//       应当从外部进行注入，比如在 World 中的对象，可以由 World 注入 WorldObjectResolver，如果没完成注入
//       则无法完成反序列化

namespace skr
{
// 用于打包一组对象，一般用于序列化/反序列化
struct ObjectPackage
{
    SKR_RC_IMPL()

private:
    // TODO. local_list: Vector<ObjectResolver> id -> Resolver，反序列化查询
    // TODO. out_list: Vector<ObjectResolver> id -> Resolver，反序列化查询
    // TODO. objects: Map<GUID, Object> object_id -> Object，对象查询
};

// 用于 Resolve 的中间人
struct ObjectResolver
{
    SKR_RC_IMPL()

private:
    RC<ObjectPackage> _package;
    GUID _object_id = {};
};
} // namespace skr