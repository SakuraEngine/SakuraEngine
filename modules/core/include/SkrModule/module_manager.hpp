#pragma once
#include "module.hpp"
#include "SkrDependencyGraph/dependency_graph.hpp"

SKR_EXTERN_C SKR_CORE_API 
skr::ModuleManager* skr_get_module_manager();

namespace skr
{
struct ModuleProperty : public DependencyGraphNode 
{
    bool bActive = false;
    bool bShared = false;
    skr::String name;
};
using module_registerer = skr::stl_function<IModule*(void)>;
class ModuleManager
{
    friend struct IModule;
public:
    ModuleManager() = default;
    virtual ~ModuleManager() = default;
    virtual IModule* get_module(const skr::String& name) = 0;
    virtual const struct ModuleGraph* make_module_graph(const skr::String& entry, bool shared = true) = 0;
    virtual bool patch_module_graph(const skr::String& name, bool shared = true, int argc = 0, char8_t** argv = nullptr) = 0;
    virtual int init_module_graph(int argc, char8_t** argv) = 0;
#if __cpp_char8_t
    inline int init_module_graph(int argc, char** argv) { return init_module_graph(argc, (char8_t**)argv); }
#endif
    virtual bool destroy_module_graph(void) = 0;
    virtual void mount(const char8_t* path) = 0;
    virtual skr::StringView get_root(void) = 0;
    virtual ModuleProperty& get_module_property(const skr::String& name) = 0;
    virtual void enable_hotfix_for_module(skr::StringView name) = 0;
    //update for hot reload
    virtual bool update(void) = 0;

    virtual void register_subsystem(const char8_t* moduleName, const char8_t* id, ModuleSubsystemBase::CreatePFN pCreate) = 0;

    virtual void registerStaticallyLinkedModule(const char8_t* moduleName, module_registerer _register) = 0;
};

template <typename ModuleClass>
struct SStaticallyLinkedModuleRegistrant {
    SStaticallyLinkedModuleRegistrant(const char8_t* InModuleName)
    {
        skr::stl_function<IModule*(void)> func =
        []() {
            return new ModuleClass();
        };
        skr_get_module_manager()->registerStaticallyLinkedModule(InModuleName, func);
    }
};
} // namespace skr
