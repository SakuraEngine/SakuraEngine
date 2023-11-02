#pragma once

namespace skr::rttr
{
struct Type;

// FIXME. 暂时以 TypeLoader 作为元信息的注册媒介
struct TypeLoader {
    virtual ~TypeLoader() = default;

    // 这里的二段式加载是为了解决 RecordType 对自身循环依赖的问题
    virtual Type* create()            = 0;
    virtual void  load(Type* type)    = 0;
    virtual void  destroy(Type* type) = 0;
};
} // namespace skr::rttr