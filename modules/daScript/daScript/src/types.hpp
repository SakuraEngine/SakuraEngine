#pragma once
#include "daScript/daScript.h"
#include "platform/memory.h"
#include "utils/log.h"

#include "SkrDAScript/text_printer.hpp"
#include "SkrDAScript/file.hpp"
#include "SkrDAScript/ctx.hpp"

namespace skr {
namespace das {

struct TextPrinterImpl : public TextPrinter
{
    TextPrinterImpl()
    {

    }

    TextPrinter printer;
};

struct FileAccessImpl : public FileAccess
{
    FileAccessImpl()
    {
        fAccess = ::das::make_smart<::das::FsFileAccess>();
    }

    bool set_text_file(const char8_t* name, const char8_t* text, uint32_t len, bool own = false) SKR_NOEXCEPT
    {
        auto fileInfo = 
            ::das::make_unique<::das::TextFileInfo>((const char*)text, len, own);
        auto finfo = fAccess->setFileInfo((const char*)name, ::das::move(fileInfo));
        return finfo;
    }

    ::das::FileAccessPtr fAccess;
};

struct ScriptContext final : public ::das::Context
{
    using Super = ::das::Context;
    ScriptContext(uint32_t stackSize) : Super(stackSize) {}
    ScriptContext(Super& ctx, uint32_t category) : Super(ctx, category) {}

    void to_out(const char* message) { SKR_LOG_INFO(message); }
    void to_err(const char* message) { SKR_LOG_ERROR(message); }
};

struct ContextImpl : public Context
{
    ~ContextImpl() SKR_NOEXCEPT {}
    class ::das::Context* get_context() SKR_NOEXCEPT override { return &ctx; }

    ScriptContext ctx;
};

} // namespace das
} // namespace skr