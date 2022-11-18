#pragma once
#include "platform/memory.h"
#include "module/module_manager.hpp"

namespace skr
{
struct RUNTIME_API ModuleSubsystem : public ModuleSubsystemBase
{
    virtual ~ModuleSubsystem() SKR_NOEXCEPT = default;
    
    template<typename this_type>
    struct Registerer
    {
        inline Registerer(const char* id, const char* module_name)
        {
            auto module_manager = ::skr_get_module_manager();
            module_manager->register_subsystem(module_name, id, &Registerer::Creater);
        }

        static ModuleSubsystemBase* Creater()
        {
            return static_cast<ModuleSubsystemBase*>(SkrNew<this_type>());
        }
    };
};
}

#define SKR_MODULE_SUBSYSTEM(Subsystem, ModuleName) static skr::ModuleSubsystem::Registerer<Subsystem> subMod##__FILE__##__LINE__(STRINGIFY(__FILE__)STRINGIFY(__LINE__), #ModuleName);