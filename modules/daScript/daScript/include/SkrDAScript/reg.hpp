#pragma once
#include "SkrDAScript/module.configure.h"
#include "platform/configure.h"
#include "utils/types.h"

namespace skr {
namespace das {

struct SKR_DASCRIPT_API Register 
{
    template<typename T> static Register store(T i32);
    template<typename T> static T load(Register r);

    template<typename T> T load() const { return Register::load<T>(*this); }
    template<typename T> void store(T v) { *this = Register::store<T>(v); }

protected:
    skr_float4_t _v;
};

#define DECL_REG_LS(type) \
template<> SKR_DASCRIPT_API type Register::load<type>(Register r); template<> SKR_DASCRIPT_API Register Register::store<type>(type v)

DECL_REG_LS(float);
DECL_REG_LS(double);
DECL_REG_LS(char);
#if defined(_MSC_VER)
DECL_REG_LS(wchar_t);
#endif
DECL_REG_LS(bool);
DECL_REG_LS(int8_t);
DECL_REG_LS(int16_t);
DECL_REG_LS(int32_t);
DECL_REG_LS(int64_t);
DECL_REG_LS(uint8_t);
DECL_REG_LS(uint16_t);
DECL_REG_LS(uint32_t);
DECL_REG_LS(uint64_t);

} // namespace das
} // namespace skr