#pragma once
#include "daScript/daScript.h"
#include "platform/memory.h"
#include "utils/log.h"

#include "SkrDAScript/text_printer.hpp"
#include "SkrDAScript/file.hpp"
#include "SkrDAScript/library.hpp"
#include "SkrDAScript/program.hpp"
#include "SkrDAScript/ctx.hpp"

namespace skr {
namespace das {

struct TextPrinterImpl : public TextPrinter
{
    TextPrinterImpl()
    {

    }

    ::das::TextPrinter printer;
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

struct LibraryImpl : public Library
{
    ::das::ModuleGroup libGroup;
};

struct ProgramImpl : public Program
{
    ProgramImpl(::das::ProgramPtr&& program) : program(::das::move(program)) {}

    uint32_t get_ctx_stack_size() SKR_NOEXCEPT;
    bool simulate(Context* ctx, TextPrinter* tout) SKR_NOEXCEPT;

    ::das::ProgramPtr program;
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
    ContextImpl(uint32_t stackSize) : ctx(stackSize) {}
    ~ContextImpl() SKR_NOEXCEPT {}
    // class ::das::Context* get_context() SKR_NOEXCEPT override { return &ctx; }

    Function find_function(const char8_t* name) SKR_NOEXCEPT;
    void eval(Function func) SKR_NOEXCEPT;

    ScriptContext ctx;
};

} // namespace das
} // namespace skr