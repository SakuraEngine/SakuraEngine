#pragma once
#include "SkrDAScript/reg.hpp"
#include "containers/text.hpp"

namespace skr {
namespace das {

struct FileAccess;
struct TextPrinter;
struct Library;
struct Program;
struct Context;

struct EnvironmentDescriptor
{
    uint32_t _nothing_ = 0;
};

struct CompileDescriptor
{
    bool aot = false;
};

struct SKR_DASCRIPT_API Environment
{
    static void Initialize(const EnvironmentDescriptor& desc) SKR_NOEXCEPT;
    static void Finalize() SKR_NOEXCEPT;
    static const skr::text::text GetRootDir() SKR_NOEXCEPT;

    virtual ~Environment() SKR_NOEXCEPT;
    static Program* compile_dascript(const char8_t* name, FileAccess* faccess, TextPrinter* tout, Library* lib, const CompileDescriptor* desc = nullptr);
};

} // namespace das
} // namespace skr