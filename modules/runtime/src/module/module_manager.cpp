#include "module/module_manager.hpp"
#include "module/subsystem.hpp"
#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/shared_library.hpp"
#include "SkrRT/misc/log.h"
#include "SkrRT/containers/hashmap.hpp"
#include "SkrRT/serde/json/reader.h"
#include "SkrRT/platform/filesystem.hpp"


#if defined(_MSC_VER)
bool cr_pdb_replace(const std::string &filename, const std::string &pdbname,
                           std::string &orig_pdb);

static bool ProcessPDB(const skr::filesystem::path& dst)
{
    auto basePath = dst.lexically_normal();
    skr::filesystem::path folder, fname, ext;
    folder = basePath.parent_path();
    fname = basePath.stem();
    ext = basePath.extension();
    //replace ext with .pdb
    auto pdbDst = folder / (fname.string() + ".pdb");
    std::string orig_pdb;
    bool result = cr_pdb_replace(dst.string(), fname.string() + ".pdb", orig_pdb);
    std::error_code ec;
    skr::filesystem::copy(orig_pdb, pdbDst, skr::filesystem::copy_options::overwrite_existing, ec);
    if(ec)
    {
        SKR_LOG_ERROR("copy pdb file failed: %s", ec.message().c_str());
        result = false;
    }
    return result;
}
#endif

namespace skr
{
struct ModuleContext
{
    skr::filesystem::path path = {};
    skr::filesystem::path temppath = {};
    skr::filesystem::file_time_type timestamp = {};
    unsigned int version = 0;
    unsigned int next_version = 1;
    unsigned int last_working_version = 0;
};
class ModuleManagerImpl : public skr::ModuleManager
{
public:
    ModuleManagerImpl()
    {
        auto sucess = processSymbolTable.load(nullptr);
        assert(sucess && "Failed to load symbol table");(void)sucess;
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
    virtual IModule* get_module(const skr::string& name) final;
    virtual const struct ModuleGraph* make_module_graph(const skr::string& entry, bool shared = true) final;
    virtual bool patch_module_graph(const skr::string& name, bool shared = true, int argc = 0, char8_t** argv = nullptr) final;
    virtual int init_module_graph(int argc, char8_t** argv) final;
    virtual bool destroy_module_graph(void) final;
    virtual void mount(const char8_t* path) final;
    virtual skr::string_view get_root(void) final;
    virtual ModuleProperty& get_module_property(const skr::string& name) final;
    virtual void enable_hotfix_for_module(skr::string_view name) override final;
    virtual bool update(void) override final;

    virtual void register_subsystem(const char8_t* moduleName, const char8_t* id, ModuleSubsystemBase::CreatePFN pCreate) final;

    virtual void registerStaticallyLinkedModule(const char8_t* moduleName, module_registerer _register) final;

protected:
    virtual IModule* spawnStaticModule(const skr::string& moduleName) final;
    virtual IModule* spawnDynamicModule(const skr::string& moduleName, bool hotfix) final;
    virtual bool loadHotfixModule(SharedLibrary& lib, const skr::string& moduleName) final;

private:
    bool __internal_DestroyModuleGraph(const skr::string& nodename);
    bool __internal_UpdateModuleGraph(const skr::string& nodename);
    void __internal_MakeModuleGraph(const skr::string& entry, bool shared = false);
    bool __internal_InitModuleGraph(const skr::string& nodename, int argc, char8_t** argv);
    ModuleInfo parseMetaData(const char8_t* metadata);

private:
    skr::string moduleDir;
    eastl::vector<skr::string> roots;
    skr::string mainModuleName;
    // ModuleGraphImpl moduleDependecyGraph;
    skr::DependencyGraph* dependency_graph = nullptr;
    skr::flat_hash_set<skr::string, skr::hash<skr::string>> hotfixTraversalSet;
    skr::flat_hash_map<skr::string, ModuleContext, skr::hash<skr::string>> hotfixModules;
    skr::flat_hash_map<skr::string, ModuleProperty*, skr::hash<skr::string>> nodeMap;
    skr::flat_hash_map<skr::string, module_registerer, skr::hash<skr::string>> initializeMap;
    skr::flat_hash_map<skr::string, eastl::unique_ptr<IModule>, skr::hash<skr::string>> modulesMap;
    skr::flat_hash_map<skr::string, eastl::vector<skr::string>, skr::hash<skr::string>> subsystemIdMap;
    skr::flat_hash_map<skr::string, eastl::vector<ModuleSubsystemBase::CreatePFN>, skr::hash<skr::string>> subsystemCreateMap;

    SharedLibrary processSymbolTable;
};

void ModuleManagerImpl::register_subsystem(const char8_t* moduleName, const char8_t* id, ModuleSubsystemBase::CreatePFN pCreate)
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

void ModuleManagerImpl::registerStaticallyLinkedModule(const char8_t* moduleName, module_registerer _register)
{
    if (initializeMap.find(moduleName) != initializeMap.end())
    {
        return;
    }
    initializeMap[moduleName] = _register;
}

IModule* ModuleManagerImpl::spawnStaticModule(const skr::string& name)
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
    SDefaultDynamicModule(const char8_t* name) : name(name) {}
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s loaded!", name.c_str());
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s executed!", name.c_str());
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_TRACE("[default implementation] dynamic module %s unloaded!", name.c_str());
    }

    skr::string name = u8"";
};

static skr::filesystem::path GetVersionPath(const skr::filesystem::path &basepath,
                                   unsigned version,
                                   const skr::filesystem::path &temppath) {
    auto basePath = basepath.lexically_normal();
    skr::filesystem::path folder, fname, ext;
    folder = basePath.parent_path();
    fname = basePath.stem();
    ext = basePath.extension();
    auto ver = std::to_string(version);
    if(!temppath.empty())
    {
        folder = temppath;
    }
    return folder / (fname.string() + ver + ext.string());
}

bool ModuleManagerImpl::loadHotfixModule(SharedLibrary& lib, const skr::string& moduleName)
{
    auto& ctx = hotfixModules[moduleName];
    skr::string filename;
    filename.append(skr::SharedLibrary::GetPlatformFilePrefixName())
            .append(moduleName)
            .append(skr::SharedLibrary::GetPlatformFileExtensionName());
    skr::filesystem::path path = filename.c_str();
    ctx.path = path;
    std::error_code ec;
    if(!skr::filesystem::exists(path, ec))
    {
        SKR_LOG_ERROR("hotfix module %s not found!", path.c_str());
        return false;
    }
    skr::filesystem::path new_path = GetVersionPath(path, ctx.version, ctx.temppath);
    {
        ctx.last_working_version = ctx.version;
        skr::filesystem::copy(path, new_path, skr::filesystem::copy_options::overwrite_existing, ec);
        if(ec)
        {
            SKR_LOG_ERROR("hotfix module %s rename failed! reason: %s", path.c_str(), ec.message().c_str());
            return false;
        }
        ctx.next_version = ctx.next_version + 1;
#if defined(_MSC_VER)
        if(!ProcessPDB(new_path))
        {
            SKR_LOG_ERROR("hotfix module %s pdb process failed, debugging may be "
                         "affected and/or reload may fail", path.c_str());
        }
#endif
    }
    if(!lib.load(new_path.u8string().c_str()))
    {
        SKR_LOG_ERROR("hotfix module %s load failed!", new_path.c_str());
        return false;
    }
    //TODO: validate sections
    //TODO: reload sections
    ctx.timestamp = skr::filesystem::last_write_time(new_path, ec);
    ctx.version = ctx.next_version - 1;
    return true;
}

IModule* ModuleManagerImpl::spawnDynamicModule(const skr::string& name, bool hotfix)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name].get();
    eastl::unique_ptr<SharedLibrary> sharedLib = eastl::make_unique<SharedLibrary>();
    skr::string initName(u8"__initializeModule");
    skr::string mName(name);
    initName.append(mName);
    // try load in program
    IModule* (*func)() = nullptr;

    skr::string metaymbolname = u8"__skr_module_meta__";
    metaymbolname.append(name);
    const bool is_proc_mod = processSymbolTable.hasSymbol(metaymbolname.u8_str());
    
    if (processSymbolTable.hasSymbol(initName.u8_str()))
    {
        func = processSymbolTable.get<IModule*()>(initName.u8_str());
    }
    if (hotfix && (is_proc_mod || func))
    {
        SKR_LOG_ERROR("Hotfix module %s failed, module already loaded!", name.c_str());
    }
#ifndef SHIPPING_ONE_ARCHIVE
    if (!is_proc_mod && func == nullptr)
    {
        // try load dll
        skr::string filename;
        filename.append(skr::SharedLibrary::GetPlatformFilePrefixName())
                .append(name)
                .append(skr::SharedLibrary::GetPlatformFileExtensionName());
        auto finalPath = (skr::filesystem::path(moduleDir.c_str()) / filename.c_str()).u8string();
        if(!hotfix)
        {
            if (!sharedLib->load((const char8_t*)finalPath.c_str()))
            {
                SKR_LOG_DEBUG("%s\nLoad Shared Lib Error:%s", filename.c_str(), sharedLib->errorString().c_str());
            }
            else
            {
                SKR_LOG_TRACE("Load dll success: %s", filename.c_str());
                if (sharedLib->hasSymbol(initName.u8_str()))
                {
                    func = sharedLib->get<IModule*()>(initName.u8_str());
                }
            }
        }
        else
        {
            if(!loadHotfixModule(*sharedLib, name))
            {
                SKR_LOG_ERROR("Hotfix module %s failed, load failed!", name.c_str());
            }
            else
            {
                SKR_LOG_TRACE("Hotfix module %s success!", name.c_str());
                if (sharedLib->hasSymbol(initName.u8_str()))
                {
                    func = sharedLib->get<IModule*()>(initName.u8_str());
                }
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
        modulesMap[name] = eastl::make_unique<SDefaultDynamicModule>(name.u8_str());
    }
    IDynamicModule* module = (IDynamicModule*)modulesMap[name].get();
    module->sharedLib = eastl::move(sharedLib);
    // pre-init name for meta reading
    module->information.name = name;
    module->information = parseMetaData(module->get_meta_data());
    return module;
}

ModuleInfo ModuleManagerImpl::parseMetaData(const char8_t* metadata)
{
    ModuleInfo info;
    auto meta = simdjson::padded_string((const char*)metadata, strlen((const char*)metadata));
    simdjson::ondemand::parser parser;
    auto doc = parser.iterate(meta);
    skr::json::Read(doc.find_field("api").value_unsafe(), info.core_version);
    skr::json::Read(doc.find_field("name").value_unsafe(), info.name);
    skr::json::Read(doc.find_field("prettyname").value_unsafe(), info.prettyname);
    skr::json::Read(doc.find_field("version").value_unsafe(), info.version);
    skr::json::Read(doc.find_field("linking").value_unsafe(), info.linking);
    skr::json::Read(doc.find_field("url").value_unsafe(), info.url);
    skr::json::Read(doc.find_field("license").value_unsafe(), info.license);
    skr::json::Read(doc.find_field("copyright").value_unsafe(), info.copyright);
    auto deps_doc = doc.find_field("dependencies");
    if (deps_doc.error() == simdjson::SUCCESS)
    {
        for (auto&& jdep : deps_doc)
        {
            ModuleDependency dep;
            skr::json::Read(jdep.find_field("name").value_unsafe(), dep.name);
            skr::json::Read(jdep.find_field("version").value_unsafe(), dep.version);
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

IModule* ModuleManagerImpl::get_module(const skr::string& name)
{
    if (modulesMap.find(name) == modulesMap.end())
        return nullptr;
    return modulesMap.find(name)->second.get();
}

ModuleProperty& ModuleManagerImpl::get_module_property(const skr::string& entry)
{
    return *nodeMap.find(entry)->second;
}

bool ModuleManagerImpl::__internal_InitModuleGraph(const skr::string& nodename, int argc, char8_t** argv)
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

bool ModuleManagerImpl::__internal_DestroyModuleGraph(const skr::string& nodename)
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

int ModuleManagerImpl::init_module_graph(int argc, char8_t** argv)
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

void ModuleManagerImpl::__internal_MakeModuleGraph(const skr::string& entry, bool shared)
{
    if (nodeMap.find(entry) != nodeMap.end())
        return;
    bool hotfix = hotfixModules.contains(entry);
    IModule* _module = shared ?
            spawnDynamicModule(entry, hotfix) :
            spawnStaticModule(entry);
    auto prop = nodeMap[entry] = SkrNew<ModuleProperty>();
    prop->name = entry;
    prop->bActive = false;
    prop->bShared = shared;
    SKR_ASSERT(hotfix <= _module->reloadable());
    dependency_graph->insert(prop);
    auto moduleInfo = _module->get_module_info();
    if (moduleInfo->dependencies.size() == 0)
        roots.push_back(entry);
    for (auto i = 0u; i < moduleInfo->dependencies.size(); i++)
    {
        auto iterName = moduleInfo->dependencies[i].name.u8_str();
        // Static
        if (initializeMap.find(iterName) != initializeMap.end())
            __internal_MakeModuleGraph(iterName, false);
        else
            __internal_MakeModuleGraph(iterName, true);

        dependency_graph->link(nodeMap[entry], nodeMap[iterName]);
    }
}

const ModuleGraph* ModuleManagerImpl::make_module_graph(const skr::string& entry, bool shared /*=false*/)
{
    mainModuleName = entry;
    __internal_MakeModuleGraph(entry, shared);
    return (struct ModuleGraph*)dependency_graph;
}

bool ModuleManagerImpl::patch_module_graph(const skr::string& entry, bool shared, int argc, char8_t** argv)
{
    __internal_MakeModuleGraph(entry, shared);
    if (!__internal_InitModuleGraph(entry, argc, argv))
        return false;
    return true;
}
void ModuleManagerImpl::enable_hotfix_for_module(skr::string_view name)
{
    std::pair<skr::string, ModuleContext> pair(name, ModuleContext{});
    hotfixModules.insert(std::move(pair));
}

bool ModuleManagerImpl::__internal_UpdateModuleGraph(const skr::string& entry)
{
    if (hotfixTraversalSet.find(entry) != hotfixTraversalSet.end())
        return true;
    dependency_graph->foreach_neighbors(nodeMap.find(entry)->second, 
        [this](DependencyGraphNode* node){
            ModuleProperty* property = static_cast<ModuleProperty*>(node);
            __internal_UpdateModuleGraph(property->name);
        });
    auto iter = hotfixModules.find(entry);
    if(iter == hotfixModules.end())
        return true;
    auto& ctx = iter->second;
    //check file timestamp
    bool changed = std::filesystem::last_write_time(ctx.path) > ctx.timestamp;
    if(!changed)
        return true;
    //reload module
    SKR_LOG_DEBUG("Hotfix module: %s", entry.c_str());
    
    eastl::unique_ptr<SharedLibrary> sharedLib = eastl::make_unique<SharedLibrary>();
    //unload old module
    auto this_module = (IHotfixModule*)get_module(entry);
    // subsystems
    for (auto&& subsystem : this_module->subsystems)
    {
        subsystem->BeginReload();
    }
    for (auto&& subsystem : this_module->subsystems)
    {
        SkrDelete(subsystem);
    }
    this_module->on_reload_begin();
    auto this_state = std::move(this_module->state);
    auto old_lib = std::move(this_module->sharedLib);
    modulesMap[entry].reset();
    subsystemCreateMap[entry].clear();
    //old_lib->unload();
    IModule* (*func)() = nullptr;
    if(!loadHotfixModule(*sharedLib, entry))
    {
        SKR_LOG_ERROR("Failed to load hotfix module: %s", entry.c_str());
        return false;
    }
    else 
    {
        skr::string initName(u8"__initializeModule");
        skr::string mName(entry);
        initName.append(mName);
        if (sharedLib->hasSymbol(initName.u8_str()))
        {
            func = sharedLib->get<IModule*()>(initName.u8_str());
        }
        if(!func)
        {
            SKR_LOG_ERROR("Failed to load hotfix module: %s", entry.c_str());
            return false;
        }
    }
    auto new_module = (IHotfixModule*)func();
    new_module->sharedLib = std::move(sharedLib);
    // pre-init name for meta reading
    new_module->information.name = entry;
    new_module->information = parseMetaData(new_module->get_meta_data());
    modulesMap[entry].reset(new_module);
    new_module->state = std::move(this_state);
    new_module->on_reload_finish();
    auto&& create_funcs = subsystemCreateMap[entry];
    for (auto&& func : create_funcs)
    {
        auto subsystem = func();
        new_module->subsystems.emplace_back(subsystem);
    }
    for (auto&& subsystem : new_module->subsystems)
    {
        subsystem->EndReload();
    }
    return true;
}

bool ModuleManagerImpl::update()
{
    return __internal_UpdateModuleGraph(mainModuleName);
}

void ModuleManagerImpl::mount(const char8_t* rootdir)
{
    moduleDir = rootdir;
}

skr::string_view ModuleManagerImpl::get_root(void)
{
    return skr::string_view(skr::string_view(moduleDir.u8_str(), (size_t)moduleDir.size()));
}

} // namespace skr