#pragma once
#include "SkrRT/module.configure.h"

namespace skr::rttr
{
struct Type;

// FIXME. 暂时以 TypeLoader 作为元信息的注册媒介
struct TypeLoader {
    virtual ~TypeLoader() = default;

    virtual Type* load()              = 0;
    virtual void  destroy(Type* type) = 0;
};
} // namespace skr::rttr