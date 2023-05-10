#pragma once
#include "daScript/daScript.h"
#include "platform/memory.h"
#include "utils/log.h"

#include "SkrDAScript/text_printer.hpp"
#include "SkrDAScript/file.hpp"
#include "SkrDAScript/library.hpp"
#include "SkrDAScript/program.hpp"
#include "SkrDAScript/ctx.hpp"
#include "SkrDAScript/annotation.hpp"
#include "SkrDAScript/module.hpp"

namespace skr {
namespace das {

struct TextPrinterImpl : public TextPrinter
{
    TextPrinterImpl()
    {

    }

    void print(const char8_t* text) SKR_NOEXCEPT;

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
    LibraryImpl(const LibraryDescriptor& desc) SKR_NOEXCEPT;
    ~LibraryImpl() SKR_NOEXCEPT;
    
    Module* get_this_module() const SKR_NOEXCEPT;

    void add_module(Module* mod) SKR_NOEXCEPT;
    void add_builtin_module() SKR_NOEXCEPT;

    ::das::ModuleGroup libGroup;
    Module* thisModule;
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

    void to_out(const char* message);
    void to_err(const char* message); 
};

struct ContextImpl : public Context
{
    ContextImpl(uint32_t stackSize) : ctx(stackSize) {}
    ~ContextImpl() SKR_NOEXCEPT {}
    // class ::das::Context* get_context() SKR_NOEXCEPT override { return &ctx; }

    SimFunctionId find_function(const char8_t* name) SKR_NOEXCEPT;
    Register eval(SimFunctionId func, Register* args = nullptr, Sequence* generated = nullptr) SKR_NOEXCEPT;
    Register eval_with_catch(SimFunctionId func, Register* args = nullptr, Sequence* generated = nullptr) SKR_NOEXCEPT;

    ScriptContext ctx;
};

struct StructureAnnotationImpl : public StructureAnnotation
{
    StructureAnnotationImpl(Library* library, const StructureAnnotationDescriptor& desc) SKR_NOEXCEPT;
    void* get_ptrptr() SKR_NOEXCEPT { return &annotation; }

    Library* get_library() const SKR_NOEXCEPT;
    void add_field(uint32_t offset, EBuiltinType type, const char8_t* na, const char8_t* cppna = nullptr) SKR_NOEXCEPT;
    void add_field(uint32_t offset, TypeDecl typedecl, const char8_t* na, const char8_t* cppna = nullptr) SKR_NOEXCEPT;

    struct Structure : public ::das::BasicStructureAnnotation
    {
        Structure(::das::ModuleLibrary* l, const StructureAnnotationDescriptor& desc) 
            : BasicStructureAnnotation((const char*)desc.name, (const char*)desc.cppname, l),
              size(desc.size), alignment(desc.alignment), flags(desc.flags),
              initializer(desc.initializer), finalizer(desc.finalizer) 
        {

        }
    
        virtual bool isSmart() const override { return false; }
        virtual bool isRawPod() const override { return false; }

        virtual size_t getSizeOf() const override { return size ? size : 1; }
        virtual size_t getAlignOf() const override { return alignment ? alignment : 1; }
        virtual bool hasNonTrivialCtor() const override { return flags & EStructureAnnotationFlag::HasNonTrivalCtor; }
        virtual bool hasNonTrivialDtor() const override { return flags & EStructureAnnotationFlag::HasNonTrivalDtor; }
        virtual bool hasNonTrivialCopy() const override { return flags & EStructureAnnotationFlag::HasNonTrivalCopy; }
        virtual bool isPod() const override { return flags & EStructureAnnotationFlag::IsPOD; }
        virtual bool canClone() const override { return flags & EStructureAnnotationFlag::CanClone; }
        virtual bool canNew() const override { return flags & EStructureAnnotationFlag::CanNew; }
        virtual bool canDeletePtr() const override { return flags & EStructureAnnotationFlag::CanDeletePtr; }

        virtual ::das::SimNode* simulateGetNew(::das::Context& context, const ::das::LineInfo& at ) const override;
        virtual ::das::SimNode* simulateDeletePtr(::das::Context& context, const ::das::LineInfo& at, ::das::SimNode* sube, uint32_t count) const override;
    
        uint64_t size = 0;
        uint64_t alignment = 0;
        StructureAnnotationFlags flags = EStructureAnnotationFlag::None;
        void (*initializer)(void* ptr) = nullptr;
        void (*finalizer)(void* ptr) = nullptr;
    };

    LibraryImpl* Lib;
    ::das::smart_ptr<Structure> annotation;
};

struct ModuleImpl : public Module
{
    ModuleImpl(const char8_t* name);
    void add_annotation(Annotation* annotation) SKR_NOEXCEPT;

    class ModuleSkr : public ::das::Module {
    public:
        ModuleSkr(const char8_t* n = u8"") : ::das::Module((const char*)n) {}
        virtual ::das::ModuleAotType aotRequire ( ::das::TextWriter & ) const { return ::das::ModuleAotType::cpp; }
    };

    ModuleSkr* mod;
};

} // namespace das
} // namespace skr