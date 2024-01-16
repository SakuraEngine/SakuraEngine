#pragma once
#include "SkrMemory/memory.h"
#include "SkrModule/module_manager.hpp"

namespace skr 
{
struct SKR_CORE_API ModuleSubsystem : public ModuleSubsystemBase {
    virtual ~ModuleSubsystem() SKR_NOEXCEPT = default;

    template <typename this_type>
    struct Registerer {
        inline Registerer(const char8_t* id, const char8_t* module_name)
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
} // namespace skr sreflect

#define SKR_MODULE_SUBSYSTEM(Subsystem, ModuleName) static skr::ModuleSubsystem::Registerer<Subsystem> subMod##__FILE__##__LINE__((const char8_t*)SKR_MAKE_STRING(__FILE__) SKR_MAKE_STRING(__LINE__), (const char8_t*)#ModuleName);