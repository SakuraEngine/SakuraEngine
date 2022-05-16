#include "module/module_manager.hpp"
#include "utils/DAG.boost.hpp"
#include "platform/memory.h"
#include "EASTL/map.h"
#include "utils/log.h"
#include "json/reader.h"
#include "ghc/filesystem.hpp"

namespace skr
{
using namespace boost;
struct ModuleProp_t {
    using kind = vertex_property_tag;
};
using ModuleProp = property<ModuleProp_t, ModuleProperty>;
using ModuleGraphImpl = skr::DAG::Graph<ModuleProp>;
using ModuleNode = skr::DAG::GraphVertex<ModuleProp>;

class ModuleManagerImpl : public skr::ModuleManager
{
public:
    ModuleManagerImpl() = default;
    ~ModuleManagerImpl() = default;
    virtual IModule* get_module(const eastl::string& name) final;
    virtual const struct ModuleGraph* make_module_graph(const eastl::string& entry, bool shared = true) final;
    virtual bool patch_module_graph(const eastl::string& name, bool shared = true) final;
    virtual bool init_module_graph(void) final;
    virtual bool destroy_module_graph(void) final;
    virtual void mount(const char8_t* path) final;
    virtual eastl::string_view get_root(void) final;
    virtual ModuleProperty get_module_property(const eastl::string& name) final;
    virtual void set_module_property(const eastl::string& name, const ModuleProperty& prop) final;

    virtual void registerStaticallyLinkedModule(const char* moduleName, module_registerer _register) final;

protected:
    virtual IModule* spawnStaticModule(const eastl::string& moduleName) final;
    virtual IModule* spawnDynamicModule(const eastl::string& moduleName) final;

private:
    bool __internal_DestroyModuleGraph(const eastl::string& nodename);
    void __internal_MakeModuleGraph(const eastl::string& entry,
    bool shared = false);
    bool __internal_InitModuleGraph(const eastl::string& nodename);
    ModuleInfo parseMetaData(const char* metadata);

private:
    eastl::string moduleDir;
    eastl::vector<eastl::string> roots;
    eastl::string mainModuleName;
    ModuleGraphImpl moduleDependecyGraph;
    eastl::map<eastl::string, int, eastl::less<>> nodeMap;
    eastl::map<eastl::string, module_registerer, eastl::less<>> initializeMap;
    eastl::map<eastl::string, eastl::unique_ptr<IModule>, eastl::less<>> modulesMap;
};

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

IModule* ModuleManagerImpl::spawnDynamicModule(const eastl::string& name)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name].get();
    eastl::unique_ptr<SharedLibrary> sharedLib = eastl::make_unique<SharedLibrary>();
    eastl::string initName("__initializeModule");
    eastl::string mName(name);
    initName.append(mName);
    eastl::string filename;
#if defined(SKR_OS_MACOSX)
    filename.append("lib").append(name);
    filename.append(".dylib");
#elif defined(SKR_OS_UNIX)
    filename.append("lib").append(name);
    filename.append(".so");
#elif defined(SKR_OS_WINDOWS)
    filename.append(name);
    filename.append(".dll");
#endif
    auto finalPath = (ghc::filesystem::path(moduleDir.c_str()) / filename.c_str()).u8string();
    if (!sharedLib->load(finalPath.c_str()))
    {
        SKR_LOG_ERROR("%s\nLoad Shared Lib Error:%s", filename.c_str(), sharedLib->errorString().c_str());
        return nullptr;
    }
#ifdef SPA_OUTPUT_LOG
    else
        std::cout << filename << ": Load dll success!" << std::endl;
#endif
    if (sharedLib->hasSymbol(initName.c_str()))
    {
        auto func = sharedLib->get<IModule*()>(initName.c_str());
        modulesMap[name] = eastl::unique_ptr<IModule>(func());
        IDynamicModule* module = (IDynamicModule*)modulesMap[name].get();
        module->sharedLib = eastl::move(sharedLib);
        // pre-init name for meta reading
        module->information.name = name;
        module->information = parseMetaData(module->get_meta_data());
        return module;
    }
    SKR_LOG_FATAL("failed to read symbol: %s", initName.c_str());
    return nullptr;
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

ModuleProperty ModuleManagerImpl::get_module_property(const eastl::string& entry)
{
    if (nodeMap.find(entry) == nodeMap.end())
        assert(0 && "Module Node not found");
    auto node = ModuleNode(nodeMap[entry]);
    return DAG::get_vertex_property<ModuleProp_t>(node, moduleDependecyGraph);
}

void ModuleManagerImpl::set_module_property(const eastl::string& entry, const ModuleProperty& prop)
{
    DAG::set_vertex_property<ModuleProp_t>(
    DAG::vertex(nodeMap.find(entry)->second,
    moduleDependecyGraph),
    moduleDependecyGraph, prop);
}

bool ModuleManagerImpl::__internal_InitModuleGraph(const eastl::string& nodename)
{
    if (get_module_property(nodename).bActive)
        return true;
    for (auto&& iter : get_module(nodename)->get_module_info()->dependencies)
    {
        if (get_module_property(iter.name).bActive)
            continue;
        if (!__internal_InitModuleGraph(iter.name))
            return false;
    }
    get_module(nodename)->on_load();
    ModuleProperty prop;
    prop.bActive = true;
    prop.name = nodename;
    set_module_property(nodename, prop);
    return true;
}

bool ModuleManagerImpl::__internal_DestroyModuleGraph(const eastl::string& nodename)
{
    if (!get_module_property(nodename).bActive)
        return true;
    auto nexts = DAG::inv_adjacent_vertices(
    ModuleNode(nodeMap.find(nodename)->second), moduleDependecyGraph);
    for (auto iter = nexts.first; iter != nexts.second; iter++)
    {
        auto name = DAG::get_vertex_property<ModuleProp_t>(*iter, moduleDependecyGraph).name;
        auto n = get_module_property(name);
        __internal_DestroyModuleGraph(n.name);
    }
    get_module(nodename)->on_unload();
    // DAG::remove_vertex(nodeMap[nodename], moduleDependecyGraph);
    ModuleProperty prop;
    prop.bActive = false;
    prop.name = get_module_property(nodename).name;
    set_module_property(nodename, prop);
    return true;
}

bool ModuleManagerImpl::init_module_graph(void)
{
    if (!__internal_InitModuleGraph(mainModuleName))
        return false;
    get_module(mainModuleName)->main_module_exec();
    return true;
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
    IModule* _module = shared ?
                       spawnDynamicModule(entry) :
                       spawnStaticModule(entry);
    nodeMap[entry] = nodeMap.size();
    ModuleProperty prop;
    prop.name = entry;
    prop.bActive = false;
    DAG::add_vertex(prop, moduleDependecyGraph);
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
        DAG::add_edge(nodeMap[entry], nodeMap[iterName], moduleDependecyGraph);
    }
}

const ModuleGraph* ModuleManagerImpl::make_module_graph(const eastl::string& entry, bool shared /*=false*/)
{
    mainModuleName = entry;
    __internal_MakeModuleGraph(entry, shared);
    return (struct ModuleGraph*)&moduleDependecyGraph;
}

bool ModuleManagerImpl::patch_module_graph(const eastl::string& entry, bool shared)
{
    __internal_MakeModuleGraph(entry, shared);
    if (!__internal_InitModuleGraph(entry))
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

extern "C" RUNTIME_API skr::ModuleManager* __stdcall skr_get_module_manager()
{
    static skr::ModuleManager* sModuleManager = SkrNew<skr::ModuleManagerImpl>();
    return sModuleManager;
}