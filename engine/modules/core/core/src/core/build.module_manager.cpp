#include "SkrCore/memory/sp.hpp"
#include "SkrCore/serialize/json_archive.hpp"
#include "SkrOS/shared_library.hpp"
#include "SkrOS/filesystem.hpp"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrCore/module/module_manager.hpp"
#include "SkrCore/module/subsystem.hpp"
#include "SkrCore/log.h"

#if defined(_MSC_VER)
bool cr_pdb_replace(const std::string& filename, const std::string& pdbname, std::string& orig_pdb);

static bool ProcessPDB(const skr::Path& dst)
{
    auto basePath = dst.normalize();
    auto folder = basePath.parent_directory();
    auto fname = basePath.basename();
    auto ext = basePath.extension();
    // replace ext with .pdb
    skr::String pdb_name = fname.string();
    pdb_name.append(u8".pdb");
    auto pdbDst = folder / pdb_name;
    std::string orig_pdb;
    std::string dst_str(reinterpret_cast<const char*>(dst.string().data()));
    std::string fname_str(reinterpret_cast<const char*>(fname.string().data()));
    bool result = cr_pdb_replace(dst_str, fname_str + ".pdb", orig_pdb);
    skr::Path orig_pdb_path(skr::String(reinterpret_cast<const char8_t*>(orig_pdb.c_str())));
    bool copy_result = skr::fs::File::copy(orig_pdb_path, pdbDst, skr::fs::CopyOptions::OverwriteExisting);
    if (!copy_result)
    {
        SKR_LOG_ERROR(u8"copy pdb file failed");
        result = false;
    }
    return result;
}
#endif

namespace skr
{
ModuleSubsystem::~ModuleSubsystem() SKR_NOEXCEPT
{
}

struct ModuleContext
{
    skr::Path path = {};
    skr::Path temppath = {};
    skr::fs::FileTime timestamp = {};
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
        assert(sucess && "Failed to load symbol table");
        (void)sucess;
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
    virtual IModule* get_module(const skr::String& name) final;
    virtual const struct ModuleGraph* make_module_graph(const skr::String& entry, bool shared = true) final;
    virtual bool patch_module_graph(const skr::String& name, bool shared = true, int argc = 0, char8_t** argv = nullptr) final;
    virtual int init_module_graph(int argc, char8_t** argv) final;
    virtual bool destroy_module_graph(void) final;
    virtual void mount(const char8_t* path) final;
    virtual skr::StringView get_root(void) final;
    virtual ModuleProperty& get_module_property(const skr::String& name) final;
    virtual void enable_hotfix_for_module(skr::StringView name) override final;
    virtual bool update(void) override final;

    virtual void register_subsystem(const char8_t* moduleName, const char8_t* id, ModuleSubsystemBase::CreatePFN pCreate) final;

    virtual void registerStaticallyLinkedModule(const char8_t* moduleName, module_registerer _register) final;

protected:
    virtual IModule* spawnStaticModule(const skr::String& moduleName) final;
    virtual IModule* spawnDynamicModule(const skr::String& moduleName, bool hotfix) final;
    virtual bool loadHotfixModule(SharedLibrary& lib, const skr::String& moduleName) final;

private:
    bool __internal_DestroyModuleGraph(const skr::String& nodename);
    bool __internal_UpdateModuleGraph(const skr::String& nodename);
    void __internal_MakeModuleGraph(const skr::String& entry, bool shared = false);
    bool __internal_InitModuleGraph(const skr::String& nodename, int argc, char8_t** argv);
    ModuleInfo parseMetaData(const char8_t* metadata);

private:
    skr::String moduleDir;
    skr::Vector<skr::String> roots;
    skr::String mainModuleName;
    // ModuleGraphImpl moduleDependecyGraph;
    skr::DependencyGraph* dependency_graph = nullptr;
    skr::FlatHashSet<skr::String, skr::Hash<skr::String>> hotfixTraversalSet;
    skr::FlatHashMap<skr::String, ModuleContext, skr::Hash<skr::String>> hotfixModules;
    skr::FlatHashMap<skr::String, ModuleProperty*, skr::Hash<skr::String>> nodeMap;
    skr::FlatHashMap<skr::String, module_registerer, skr::Hash<skr::String>> initializeMap;
    skr::FlatHashMap<skr::String, IModule*, skr::Hash<skr::String>> modulesMap;
    skr::FlatHashMap<skr::String, skr::Vector<skr::String>, skr::Hash<skr::String>> subsystemIdMap;
    skr::FlatHashMap<skr::String, skr::Vector<ModuleSubsystemBase::CreatePFN>, skr::Hash<skr::String>> subsystemCreateMap;

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
    subsystemCreateMap[moduleName].add(pCreate);
    subsystemIdMap[moduleName].add(id);
}

void ModuleManagerImpl::registerStaticallyLinkedModule(const char8_t* moduleName, module_registerer _register)
{
    if (initializeMap.find(moduleName) != initializeMap.end())
    {
        return;
    }
    initializeMap[moduleName] = _register;
}

IModule* ModuleManagerImpl::spawnStaticModule(const skr::String& name)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name];
    if (initializeMap.find(name) == initializeMap.end())
        return nullptr;
    auto func = initializeMap[name];
    modulesMap[name] = func();
    modulesMap[name]->information = parseMetaData(modulesMap[name]->get_meta_data());
    // Delay onload call to initialize time(with dependency graph)
    // modulesMap[name]->OnLoad();
    return modulesMap[name];
}

class SDefaultDynamicModule : public skr::IDynamicModule
{
public:
    SDefaultDynamicModule(const char8_t* name)
        : name(name)
    {
    }
    virtual void on_load(int argc, char8_t** argv) override
    {
        SKR_LOG_TRACE(u8"[default implementation] dynamic module %s loaded!", name.c_str());
    }
    virtual int main_module_exec(int argc, char8_t** argv) override
    {
        SKR_LOG_TRACE(u8"[default implementation] dynamic module %s executed!", name.c_str());
        return 0;
    }
    virtual void on_unload() override
    {
        SKR_LOG_TRACE(u8"[default implementation] dynamic module %s unloaded!", name.c_str());
    }

    skr::String name = u8"";
};

static skr::Path GetVersionPath(const skr::Path& basepath, unsigned version, const skr::Path& temppath)
{
    auto basePath = basepath.normalize();
    auto folder = basePath.parent_directory();
    auto fname = basePath.basename();
    auto ext = basePath.extension();
    auto ver = std::to_string(version);
    if (!temppath.is_empty())
    {
        folder = temppath;
    }
    skr::String result_str = fname.string();
    result_str.append(skr::String(reinterpret_cast<const char8_t*>(ver.c_str())));
    result_str.append(ext.string());
    return folder / result_str;
}

bool ModuleManagerImpl::loadHotfixModule(SharedLibrary& lib, const skr::String& moduleName)
{
    auto& ctx = hotfixModules[moduleName];
    skr::String filename;
    filename.append(skr::SharedLibrary::GetPlatformFilePrefixName());
    filename.append(moduleName);
    filename.append(skr::SharedLibrary::GetPlatformFileExtensionName());
    skr::Path path(skr::String(reinterpret_cast<const char8_t*>(filename.c_str())));
    ctx.path = path;
    if (!skr::fs::File::exists(path))
    {
        SKR_LOG_ERROR(u8"hotfix module %s not found!", path.string().data());
        return false;
    }
    skr::Path new_path = GetVersionPath(path, ctx.version, ctx.temppath);
    {
        ctx.last_working_version = ctx.version;
        bool copy_result = skr::fs::File::copy(path, new_path, skr::fs::CopyOptions::OverwriteExisting);
        if (!copy_result)
        {
            SKR_LOG_ERROR(u8"hotfix module %s rename failed!", path.string().data());
            return false;
        }
        ctx.next_version = ctx.next_version + 1;
#if defined(_MSC_VER)
        if (!ProcessPDB(new_path))
        {
            SKR_LOG_ERROR(u8"hotfix module %s pdb process failed, debugging may be "
                          "affected and/or reload may fail",
                          path.string().data());
        }
#endif
    }
    if (!lib.load(new_path.string().data()))
    {
        SKR_LOG_ERROR(u8"hotfix module %s load failed!", new_path.string().data());
        return false;
    }
    // TODO: validate sections
    // TODO: reload sections
    auto info = skr::fs::File::get_info(new_path);
    ctx.timestamp = info.last_write_time;
    ctx.version = ctx.next_version - 1;
    return true;
}

IModule* ModuleManagerImpl::spawnDynamicModule(const skr::String& name, bool hotfix)
{
    if (modulesMap.find(name) != modulesMap.end())
        return modulesMap[name];
    auto sharedLib = new SharedLibrary();
    skr::String initName(u8"__initializeModule");
    skr::String mName(name);
    initName.append(mName);
    // try load in program
    IModule* (*func)() = nullptr;

    skr::String metaymbolname = u8"__skr_module_meta__";
    metaymbolname.append(name);
    const bool is_proc_mod = processSymbolTable.hasSymbol(metaymbolname.c_str());

    if (processSymbolTable.hasSymbol(initName.c_str()))
    {
        func = processSymbolTable.get<IModule*()>(initName.c_str());
    }
    if (hotfix && (is_proc_mod || func))
    {
        SKR_LOG_ERROR(u8"Hotfix module %s failed, module already loaded!", name.c_str());
    }
#ifndef SHIPPING_ONE_ARCHIVE
    if (!is_proc_mod && func == nullptr)
    {
        // try load dll
        skr::String filename;
        filename.append(skr::SharedLibrary::GetPlatformFilePrefixName());
        filename.append(name);
        filename.append(skr::SharedLibrary::GetPlatformFileExtensionName());
        skr::Path moduleDir_path(skr::String(reinterpret_cast<const char8_t*>(moduleDir.c_str())));
        skr::Path filename_path(skr::String(reinterpret_cast<const char8_t*>(filename.c_str())));
        auto finalPath = (moduleDir_path / filename_path).string();
        if (!hotfix)
        {
            if (!sharedLib->load(finalPath.data()))
            {
                SKR_LOG_DEBUG(u8"%s\nLoad Shared Lib Error:%s", filename.c_str(), sharedLib->errorString().c_str());
            }
            else
            {
                SKR_LOG_TRACE(u8"Load dll success: %s", filename.c_str());
                if (sharedLib->hasSymbol(initName.c_str()))
                {
                    func = sharedLib->get<IModule*()>(initName.c_str());
                }
            }
        }
        else
        {
            if (!loadHotfixModule(*sharedLib, name))
            {
                SKR_LOG_ERROR(u8"Hotfix module %s failed, load failed!", name.c_str());
            }
            else
            {
                SKR_LOG_TRACE(u8"Hotfix module %s success!", name.c_str());
                if (sharedLib->hasSymbol(initName.c_str()))
                {
                    func = sharedLib->get<IModule*()>(initName.c_str());
                }
            }
        }
    }
#endif
    if (func)
    {
        modulesMap[name] = func();
    }
    else
    {
        SKR_LOG_TRACE(u8"no user defined symbol: %s", initName.c_str());
        modulesMap[name] = new SDefaultDynamicModule(name.c_str());
    }
    IDynamicModule* module = (IDynamicModule*)modulesMap[name];
    module->sharedLib = sharedLib;
    // pre-init name for meta reading
    module->information.name = name;
    module->information = parseMetaData(module->get_meta_data());
    return module;
}

ModuleInfo ModuleManagerImpl::parseMetaData(const char8_t* metadata)
{
    ModuleInfo info;
    auto reader = skr::ArReadJson::ReadBuffer(metadata, std::char_traits<char8_t>::length(metadata));
    {
        skr::Archive::ObjectScope obj_scope{ reader };
        SKR_FAST_CHECK(obj_scope.is_success(), {});

        SKR_FAST_CHECK(reader.key_value(u8"api", info.core_version), {});
        SKR_FAST_CHECK(reader.key_value(u8"name", info.name), {});
        SKR_FAST_CHECK(reader.key_value(u8"prettyname", info.prettyname), {});
        SKR_FAST_CHECK(reader.key_value(u8"version", info.version), {});
        SKR_FAST_CHECK(reader.key_value(u8"linking", info.linking), {});

        SKR_FAST_CHECK(reader.key(u8"dependencies"), {});
        {
            skr::Archive::ArrayScope arr_scope{ reader };
            SKR_FAST_CHECK(arr_scope.is_success(), {});

            uint64_t arr_count;
            SKR_FAST_CHECK(reader.array_size(arr_count), {});
            info.dependencies.reserve(arr_count);

            for (uint64_t i = 0; i < arr_count; i++)
            {
                auto& dep = info.dependencies.add_default().ref();

                skr::Archive::ObjectScope dep_obj_scope{ reader };
                SKR_FAST_CHECK(dep_obj_scope.is_success(), {});
                SKR_FAST_CHECK(reader.key_value(u8"name", dep.name), {});
                SKR_FAST_CHECK(reader.key_value(u8"version", dep.version), {});
                SKR_FAST_CHECK(reader.key_value(u8"kind", dep.kind), {});
            }
        }
    }
    return info;
}

IModule* ModuleManagerImpl::get_module(const skr::String& name)
{
    if (modulesMap.find(name) == modulesMap.end())
        return nullptr;
    return modulesMap.find(name)->second;
}

ModuleProperty& ModuleManagerImpl::get_module_property(const skr::String& entry)
{
    return *nodeMap.find(entry)->second;
}

bool ModuleManagerImpl::__internal_InitModuleGraph(const skr::String& nodename, int argc, char8_t** argv)
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
        this_module->subsystems.add(subsystem);
    }
    for (auto&& subsystem : this_module->subsystems)
    {
        subsystem->Initialize();
    }
    nodeMap[nodename]->bActive = true;
    nodeMap[nodename]->name = nodename;
    return true;
}

bool ModuleManagerImpl::__internal_DestroyModuleGraph(const skr::String& nodename)
{
    if (!get_module_property(nodename).bActive)
        return true;
    auto node = nodeMap.find(nodename)->second;
    dependency_graph->foreach_inv_neighbors(node, [this](DependencyGraphNode* node) {
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
    if (modulesMap[nodename] != nullptr)
    {
        delete modulesMap[nodename];
        modulesMap[nodename] = nullptr;
    }
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

void ModuleManagerImpl::__internal_MakeModuleGraph(const skr::String& entry, bool shared)
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
        roots.add(entry);
    for (auto i = 0u; i < moduleInfo->dependencies.size(); i++)
    {
        const auto& depInfo = moduleInfo->dependencies[i];
        auto iterName = depInfo.name.c_str();
        bool isShared = depInfo.kind == u8"shared";
        __internal_MakeModuleGraph(iterName, isShared);

        auto _this = nodeMap[entry];
        auto dep = nodeMap[iterName];
        dependency_graph->link(_this, dep);
    }
}

const ModuleGraph* ModuleManagerImpl::make_module_graph(const skr::String& entry, bool shared /*=false*/)
{
    mainModuleName = entry;
    __internal_MakeModuleGraph(entry, shared);
    return (struct ModuleGraph*)dependency_graph;
}

bool ModuleManagerImpl::patch_module_graph(const skr::String& entry, bool shared, int argc, char8_t** argv)
{
    __internal_MakeModuleGraph(entry, shared);
    if (!__internal_InitModuleGraph(entry, argc, argv))
        return false;
    return true;
}
void ModuleManagerImpl::enable_hotfix_for_module(skr::StringView name)
{
    std::pair<skr::String, ModuleContext> pair(name, ModuleContext{});
    hotfixModules.insert(std::move(pair));
}

bool ModuleManagerImpl::__internal_UpdateModuleGraph(const skr::String& entry)
{
    if (hotfixTraversalSet.find(entry) != hotfixTraversalSet.end())
        return true;
    auto node = nodeMap.find(entry)->second;
    dependency_graph->foreach_neighbors(node, [this](DependencyGraphNode* node) {
        ModuleProperty* property = static_cast<ModuleProperty*>(node);
        __internal_UpdateModuleGraph(property->name);
    });
    auto iter = hotfixModules.find(entry);
    if (iter == hotfixModules.end())
        return true;
    auto& ctx = iter->second;
    // check file timestamp
    auto info = skr::fs::File::get_info(ctx.path);
    bool changed = info.last_write_time > ctx.timestamp;
    if (!changed)
        return true;
    // reload module
    SKR_LOG_DEBUG(u8"Hotfix module: %s", entry.c_str());

    auto sharedLib = new SharedLibrary();
    // unload old module
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
    [[maybe_unused]] auto old_lib = std::move(this_module->sharedLib);
    if (modulesMap[entry] != nullptr)
    {
        delete modulesMap[entry];
        modulesMap[entry] = nullptr;
    }
    subsystemCreateMap[entry].clear();
    // old_lib->unload();
    IModule* (*func)() = nullptr;
    if (!loadHotfixModule(*sharedLib, entry))
    {
        SKR_LOG_ERROR(u8"Failed to load hotfix module: %s", entry.c_str());
        return false;
    }
    else
    {
        skr::String initName(u8"__initializeModule");
        skr::String mName(entry);
        initName.append(mName);
        if (sharedLib->hasSymbol(initName.c_str()))
        {
            func = sharedLib->get<IModule*()>(initName.c_str());
        }
        if (!func)
        {
            SKR_LOG_ERROR(u8"Failed to load hotfix module: %s", entry.c_str());
            return false;
        }
    }
    auto new_module = (IHotfixModule*)func();
    new_module->sharedLib = std::move(sharedLib);
    // pre-init name for meta reading
    new_module->information.name = entry;
    new_module->information = parseMetaData(new_module->get_meta_data());
    modulesMap[entry] = new_module;
    new_module->state = std::move(this_state);
    new_module->on_reload_finish();
    auto&& create_funcs = subsystemCreateMap[entry];
    for (auto&& func : create_funcs)
    {
        auto subsystem = func();
        new_module->subsystems.add(subsystem);
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

skr::StringView ModuleManagerImpl::get_root(void)
{
    return skr::StringView(skr::StringView(moduleDir.c_str(), (size_t)moduleDir.size()));
}

} // namespace skr

SKR_EXTERN_C SKR_CORE_API skr::ModuleManager* skr_get_module_manager()
{
    static auto sModuleManager = skr::SP<skr::ModuleManagerImpl>::New();
    return sModuleManager.get();
}
