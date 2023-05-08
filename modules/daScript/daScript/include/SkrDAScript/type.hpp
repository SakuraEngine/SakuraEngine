#pragma once
#include "SkrDAScript/env.hpp"

namespace skr {
namespace das {

struct SKR_DASCRIPT_API TypeDecl
{
    virtual ~TypeDecl() SKR_NOEXCEPT;

    template<typename T>
    static TypeDecl* Create(Library* lib, const char8_t* name) SKR_NOEXCEPT
    {
        return CreateHandle(lib, name);
    }
    static void Free(TypeDecl* decl) SKR_NOEXCEPT;    

protected:
    static TypeDecl* CreateHandle(Library* lib, const char8_t* name) SKR_NOEXCEPT;
};

} // namespace das
} // namespace skr