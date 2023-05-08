#include "SkrDAScript/module.hpp"
#include "types.hpp"

namespace skr {
namespace das {

Module* Module::Create(const char8_t* name) SKR_NOEXCEPT
{
    return SkrNew<ModuleImpl>(name);
}

void Module::Free(Module* mod) SKR_NOEXCEPT
{
    SkrDelete(mod);
}

Module::~Module() SKR_NOEXCEPT
{
    // we do not need to delete mod because Environment will delete it at exit.
}

ModuleImpl::ModuleImpl(const char8_t* name)
    : mod(new ModuleSkr(name))
{

}

void ModuleImpl::add_annotation(Annotation* annotation) SKR_NOEXCEPT
{
    mod->addAnnotation(
       * static_cast<::das::smart_ptr<::das::Annotation>*>(annotation->get_ptrptr())
    );
}

} // namespace das
} // namespace skr