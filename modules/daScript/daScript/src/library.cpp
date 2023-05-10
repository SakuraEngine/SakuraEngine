#include "types.hpp"

namespace skr {
namespace das {

Library::~Library() SKR_NOEXCEPT
{

}

Library* Library::Create(const LibraryDescriptor& desc) SKR_NOEXCEPT
{
    return SkrNew<LibraryImpl>(desc);
}

void Library::Free(Library* library) SKR_NOEXCEPT
{
    SkrDelete(library);
}

struct InnerModuleImpl : public Module
{
    InnerModuleImpl(::das::Module* mod) : mod(mod) {}
    void add_annotation(Annotation* annotation) SKR_NOEXCEPT
    {
        mod->addAnnotation(
        *static_cast<::das::smart_ptr<::das::Annotation>*>(annotation->get_ptrptr())
        );
    }
    ::das::Module* mod;
};

LibraryImpl::LibraryImpl(const LibraryDescriptor& desc) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < desc.init_mod_counts; i++)
    {
        auto Mod = static_cast<ModuleImpl*>(desc.init_mods[i]);
        libGroup.addModule(Mod->mod);
    }
    libGroup.addBuiltInModule();
    thisModule = SkrNew<InnerModuleImpl>(libGroup.getThisModule());
}

LibraryImpl::~LibraryImpl() SKR_NOEXCEPT
{
    SkrDelete(thisModule);
}

Module* LibraryImpl::get_this_module() const SKR_NOEXCEPT
{
    return thisModule;
}

void LibraryImpl::add_module(Module* mod) SKR_NOEXCEPT
{
    auto Mod = static_cast<ModuleImpl*>(mod);
    libGroup.addModule(Mod->mod);
}

void LibraryImpl::add_builtin_module() SKR_NOEXCEPT
{
    libGroup.addBuiltInModule();
}

} // namespace das
} // namespace skr