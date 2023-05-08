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

LibraryImpl::LibraryImpl(const LibraryDescriptor& desc) SKR_NOEXCEPT
{
    for (uint32_t i = 0; i < desc.init_mod_counts; i++)
    {
        auto Mod = static_cast<ModuleImpl*>(desc.init_mods[i]);
        libGroup.addModule(Mod->mod);
    }
    libGroup.addBuiltInModule();
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