#include "module/module_manager.hpp"
#include "platform/memory.h"
#include "containers/hashmap.hpp"
#include "platform/shared_library.hpp"
#include "utils/log.h"
#include "json/reader.h"
#include "platform/filesystem.hpp"

namespace skr
{
class ModuleManagerImpl : public skr::ModuleManager
{
public:
    ModuleManagerImpl()
    {
        auto sucess = processSymbolTable.load(nullptr);
        assert(sucess && "Failed to load symbol table");
        dependency_graph = skr::DependencyGraph::Create();
    }
    ~ModuleManagerImpl()
    {
        for (auto&& iter : nodeMap)
        {
            SkrDelete(iter.second);
        }
        skr::DependencyGraph::Destroy(dependency_graph);
    }
    virtual IModule* get_module(const eastl::string& name) final;
    virtual const struct ModuleGraph* make_module_graph(const eastl::string& entry, bool shared = true) final;
    virtual bool patch_module_graph(const eastl::string& name, bool shared = true, int argc = 0, char** argv = nullptr) final;
    virtual int init_module_graph(int argc, char** argv) final;
    virtual bool destroy_module_graph(void) final;
    virtual void mount(const char8_t* path) final;
    virtual eastl::string_view get_root(void) final;
    virtual ModuleProperty& get_module_property(const eastl::string& name) final;

    virtual void register_subsystem(const char* moduleName, const char* id, ModuleSubsystemBase::CreatePFN pCreate) final;

    virtual void registerStaticallyLinkedModule(const char* moduleName, module_registerer _register) final;

protected:
    virtual IModule* spawnStaticModule(const eastl::string& moduleName) final;
    virtual IModule* spawnDynamicModule(const eastl::string& moduleName) final;

private:
    bool __internal_DestroyModuleGraph(const eastl::string& nodename);
    void __internal_MakeModuleGraph(const eastl::string& entry,
    bool shared = false);
    bool __internal_InitModuleGraph(const eastl::string& nodename, int argc, char** argv);
    ModuleInfo parseMetaData(const char* metadata);

private:
    eastl::string moduleDir;
    eastl::vector<eastl::string> roots;
    eastl::string mainModuleName;
    // ModuleGraphImpl moduleDependecyGraph;
    skr::DependencyGraph* dependency_graph = nullptr;
    skr::flat_hash_map<eastl::string, ModuleProperty*, eastl::hash<eastl::string>> nodeMap;
    skr::flat_hash_map<eastl::string, module_registerer, eastl::hash<eastl::string>> initializeMap;
    skr::flat_hash_map<eastl::string, eastl::unique_ptr<IModule>, eastl::hash<eastl::string>> modulesMap;
    skr::flat_hash_map<eastl::string, eastl::vector<eastl::string>, eastl::hash<eastl::string>> subsystemIdMap;
    skr::flat_hash_map<eastl::string, eastl::vector<ModuleSubsystemBase::CreatePFN>, eastl::hash<eastl::string>> subsystemCreateMap;

    SharedLibrary processSymbolTable;
};

void ModuleManagerImpl::register_subsystem(const char* moduleName, const char* id, ModuleSubsystemBase::CreatePFN pCreate)
{
    for (auto pfn : subsystemCreateMap[moduleName])
    {
        if (pfn == pCreate) return;
    }
    for (auto ID : subsystemIdMap[moduleName])
    {
        if (ID == id) return;
    }
    subsystemCreateMap[moduleName].emplace_back(pCreate);
    subsystemIdMap[moduleName].emplace_back(id);
}

void ModuleManagerImpl::registerStaticallyLinkedModule(const char* moduleName, module_registerer _register)
{
    if (initializeMap.find(moduleName) != initializeMap.end())
    {
        return;
    }
    initializeMap[moduleName] = _register;
}

IModule* ModuleManagerImpl::spawnStaticModule(const eastl::string& name)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name].get();
    if (initializeMap.find(name) == initializeMap.end())
        return nullptr;
    auto func = initializeMap[name];
    modulesMap[name] = func();
    modulesMap[name]->information = parseMetaData(modulesMap[name]->get_meta_data());
    // Delay onload call to initialize time(with dependency graph)
    // modulesMap[name]->OnLoad();
    return modulesMap[name].get();
}

class SDefaultDynamicModule : public skr::IDynamicModule
{
public:
    SDefaultDynamicModule(const char* name) : name(name) {}
    virtual void on_load(int argc, char** argv) override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s loaded!", name.c_str());
    }
    virtual int main_module_exec(int argc, char** argv) override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s executed!", name.c_str());
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s unloaded!", name.c_str());
    }

    eastl::string name = "";
};

IModule* ModuleManagerImpl::spawnDynamicModule(const eastl::string& name)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name].get();
    eastl::unique_ptr<SharedLibrary> sharedLib = eastl::make_unique<SharedLibrary>();
    eastl::string initName("__initializeModule");
    eastl::string mName(name);
    initName.append(mName);
    // try load in program
    IModule* (*func)() = nullptr;
    if (processSymbolTable.hasSymbol(initName.c_str()))
    {
        func = processSymbolTable.get<IModule*()>(initName.c_str());
    }
#ifndef MODULE_AS_OBJECTS
    if (func == nullptr)
    {
        // try load dll
        eastl::string filename;
        filename.append(skr::SharedLibrary::GetPlatformFilePrefixName())
                .append(name)
                .append(skr::SharedLibrary::GetPlatformFileExtensionName());
        auto finalPath = (skr::filesystem::path(moduleDir.c_str()) / filename.c_str()).u8string();
        if (!sharedLib->load(finalPath.c_str()))
        {
            SKR_LOG_DEBUG("%s\nLoad Shared Lib Error:%s", filename.c_str(), sharedLib->errorString().c_str());
        }
        else
        {
            SKR_LOG_TRACE("Load dll success: %s", filename.c_str());
            if (sharedLib->hasSymbol(initName.c_str()))
            {
                func = sharedLib->get<IModule*()>(initName.c_str());
            }
        }
    }
#endif
    if (func)
    {
        modulesMap[name] = eastl::unique_ptr<IModule>(func());
    }
    else
    {
        SKR_LOG_TRACE("no user defined symbol: %s", initName.c_str());
        modulesMap[name] = eastl::make_unique<SDefaultDynamicModule>(name.c_str());
    }
    IDynamicModule* module = (IDynamicModule*)modulesMap[name].get();
    module->sharedLib = eastl::move(sharedLib);
    // pre-init name for meta reading
    module->information.name = name;
    module->information = parseMetaData(module->get_meta_data());
    return module;
}

ModuleInfo ModuleManagerImpl::parseMetaData(const char* metadata)
{
    ModuleInfo info;
    auto meta = simdjson::padded_string(metadata, strlen(metadata));
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(meta);
    skr::json::Read(doc["api"].value_unsafe(), info.core_version);
    skr::json::Read(doc["name"].value_unsafe(), info.name);
    skr::json::Read(doc["prettyname"].value_unsafe(), info.prettyname);
    skr::json::Read(doc["url"].value_unsafe(), info.url);
    skr::json::Read(doc["copyright"].value_unsafe(), info.copyright);
    skr::json::Read(doc["license"].value_unsafe(), info.license);
    skr::json::Read(doc["version"].value_unsafe(), info.version);
    skr::json::Read(doc["linking"].value_unsafe(), info.linking);
    auto deps_doc = doc["dependencies"];
    if (deps_doc.error() == simdjson::SUCCESS)
    {
        for (auto&& jdep : deps_doc)
        {
            ModuleDependency dep;
            skr::json::Read(jdep["name"].value_unsafe(), dep.name);
            skr::json::Read(jdep["version"].value_unsafe(), dep.version);
            info.dependencies.emplace_back(dep);
        }
    }
    else
    {
        SKR_LOG_FATAL("parse module meta error!");
        abort();
    }
    return info;
}

IModule* ModuleManagerImpl::get_module(const eastl::string& name)
{
    if (modulesMap.find(name) == modulesMap.end())
        return nullptr;
    return modulesMap.find(name)->second.get();
}

ModuleProperty& ModuleManagerImpl::get_module_property(const eastl::string& entry)
{
    return *nodeMap.find(entry)->second;
}

bool ModuleManagerImpl::__internal_InitModuleGraph(const eastl::string& nodename, int argc, char** argv)
{
    if (get_module_property(nodename).bActive)
        return true;
    for (auto&& iter : get_module(nodename)->get_module_info()->dependencies)
    {
        if (get_module_property(iter.name).bActive)
            continue;
        if (!__internal_InitModuleGraph(iter.name, argc, argv))
            return false;
    }
    auto this_module = get_module(nodename);
    this_module->on_load(argc, argv);
    // subsystems
    auto&& create_funcs = subsystemCreateMap[nodename];
    for (auto&& func : create_funcs)
    {
        auto subsystem = func();
        this_module->subsystems.emplace_back(subsystem);
    }
    for (auto&& subsystem : this_module->subsystems)
    {
        subsystem->Initialize();
    }
    nodeMap[nodename]->bActive = true;
    nodeMap[nodename]->name = nodename;
    return true;
}

bool ModuleManagerImpl::__internal_DestroyModuleGraph(const eastl::string& nodename)
{
    if (!get_module_property(nodename).bActive)
        return true;
    dependency_graph->foreach_inv_neighbors(nodeMap.find(nodename)->second, 
        [this](DependencyGraphNode* node){
            ModuleProperty* property = static_cast<ModuleProperty*>(node);
            __internal_DestroyModuleGraph(property->name);
        });
    auto this_module = get_module(nodename);
    // subsystems
    for (auto&& subsystem : this_module->subsystems)
    {
        subsystem->Finalize();
    }
    for (auto&& subsystem : this_module->subsystems)
    {
        SkrDelete(subsystem);
    }
    this_module->on_unload();
    modulesMap[nodename].reset();
    nodeMap[nodename]->bActive = false;
    nodeMap[nodename]->name = nodename;
    return true;
}

int ModuleManagerImpl::init_module_graph(int argc, char** argv)
{
    if (!__internal_InitModuleGraph(mainModuleName, argc, argv))
        return -1;
    return get_module(mainModuleName)->main_module_exec(argc, argv);
}

bool ModuleManagerImpl::destroy_module_graph(void)
{
    for (auto& iter : roots)
    {
        if (!__internal_DestroyModuleGraph(iter))
            return false;
    }
    return true;
}

void ModuleManagerImpl::__internal_MakeModuleGraph(const eastl::string& entry, bool shared)
{
    if (nodeMap.find(entry) != nodeMap.end())
        return;
    IModule* _module = nullptr;
    if (!_module)
    {
        _module = shared ?
                  spawnDynamicModule(entry) :
                  spawnStaticModule(entry);
    }
    auto prop = nodeMap[entry] = SkrNew<ModuleProperty>();
    prop->name = entry;
    prop->bActive = false;
    dependency_graph->insert(prop);
    if (_module->get_module_info()->dependencies.size() == 0)
        roots.push_back(entry);
    for (auto i = 0u; i < _module->get_module_info()->dependencies.size(); i++)
    {
        const char* iterName = _module->get_module_info()->dependencies[i].name.c_str();
        // Static
        if (initializeMap.find(iterName) != initializeMap.end())
            __internal_MakeModuleGraph(iterName, false);
        else
            __internal_MakeModuleGraph(iterName, true);

        dependency_graph->link(nodeMap[entry], nodeMap[iterName]);
    }
}

const ModuleGraph* ModuleManagerImpl::make_module_graph(const eastl::string& entry, bool shared /*=false*/)
{
    mainModuleName = entry;
    __internal_MakeModuleGraph(entry, shared);
    return (struct ModuleGraph*)dependency_graph;
}

bool ModuleManagerImpl::patch_module_graph(const eastl::string& entry, bool shared, int argc, char** argv)
{
    __internal_MakeModuleGraph(entry, shared);
    if (!__internal_InitModuleGraph(entry, argc, argv))
        return false;
    return true;
}

void ModuleManagerImpl::mount(const char8_t* rootdir)
{
    moduleDir = rootdir;
}

eastl::string_view ModuleManagerImpl::get_root(void)
{
    return eastl::string_view(moduleDir);
}

} // namespace skr