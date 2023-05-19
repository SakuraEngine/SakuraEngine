#include "containers/hashmap.hpp"
#include "lua/skr_lua.h"
#include "platform/memory.h"
#include "misc/defer.hpp"
#include "misc/types.h"
#include "platform/guid.hpp"
#include "resource/resource_handle.h"
#include "lua/bind.hpp"
#include "misc/log.h"
#include "platform/vfs.h"
#include "ecs/dual.h"
extern "C"
{
#include "luacode.h"
}

#include "containers/string.hpp"
#include <EASTL/string.h>
#include <EASTL/string_view.h>

namespace skr::lua
{
void bind_skr_guid(lua_State* L);
void bind_skr_resource_handle(lua_State* L);
void bind_skr_log(lua_State* L);
void bind_unknown(lua_State* L);
void bind_ecs(lua_State* L);
} // namespace skr::lua

struct skr_lua_state_extra_t
{
    skr_vfs_t* vfs;
};

void replaceAll(eastl::u8string& str, const eastl::u8string_view& from, const eastl::u8string_view& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = eastl::u8string_view(str).find(from, start_pos)) != eastl::string::npos) {
        str.replace(start_pos, from.length(), to.data(), to.length());
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

static skr::flat_hash_map<lua_State*, void*> ExtraSpace;
void** lua_getextraspace(lua_State* L)
{
    return &ExtraSpace[L];
}

int skr_lua_loadfile(lua_State* L, const char* filename)
{
    skr_lua_state_extra_t* extra = (skr_lua_state_extra_t*)*(void**)lua_getextraspace(L);
    eastl::u8string path = (const char8_t*)filename;
    replaceAll(path, u8".", u8"/");
    eastl::u8string_view exts[] = { u8".lua", u8".luac" };
    skr_vfile_t* file = nullptr;
    for(int i=0; i<2; ++i)
    {
        eastl::u8string fullpath = path + exts[i].data();
        file = skr_vfs_fopen(extra->vfs, (const char8_t*)fullpath.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        if(file)
            break;
    }
    if(!file)
    {
        SKR_LOG_ERROR("[lua] Failed to open file: %s", filename);
        return 0;
    }
    SKR_DEFER({ skr_vfs_fclose(file); });
    auto size = skr_vfs_fsize(file);
    eastl::vector<char> buffer(size);
    skr_vfs_fread(file, buffer.data(), 0, size);
    auto name = eastl::u8string(u8"@") + (const char8_t*)filename;
    size_t bytecodeSize = 0;
    char* bytecode = luau_compile(buffer.data(), size, NULL, &bytecodeSize);
    SKR_DEFER({ free(bytecode); });
    if(luau_load(L, (char*)name.c_str(), bytecode, bytecodeSize, 0)==0) {
        return 1;
    }
    else {
        const char* err = lua_tostring(L,-1);
        SKR_LOG_ERROR("[lua] Failed to load file: %s", err);
        lua_pop(L,1);
        lua_pushnil(L);
        return 1;
    }
}

int skr_load_file(lua_State* L) {
    const char* fn = luaL_checkstring(L, 1);
    return skr_lua_loadfile(L, fn);
}

#define LUA_QL(x)	"'" x "'"
#define LUA_QS		LUA_QL("%s")
static const int sentinel_ = 0;
#define sentinel	((void *)&sentinel_)
static int ll_require (lua_State *L) {
  const char *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* _LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_getfield(L, 2, name);
  if (lua_toboolean(L, -1)) {  /* is it there? */
    if (lua_touserdata(L, -1) == sentinel)  /* check loops */
      luaL_error(L, "loop or previous error loading module " LUA_QS, name);
    return 1;  /* package is already loaded */
  }
  skr_load_file(L);
  lua_pushlightuserdata(L, sentinel);
  lua_setfield(L, 2, name);  /* _LOADED[name] = sentinel */
  lua_pushstring(L, name);  /* pass name as argument to module */
  lua_call(L, 1, 1);  /* run loaded module */
  if (!lua_isnil(L, -1))  /* non-nil return? */
    lua_setfield(L, 2, name);  /* _LOADED[name] = returned value */
  lua_getfield(L, 2, name);
  if (lua_touserdata(L, -1) == sentinel) {   /* module did not set a value? */
    lua_pushboolean(L, 1);  /* use true as result */
    lua_pushvalue(L, -1);  /* extra copy to be returned */
    lua_setfield(L, 2, name);  /* _LOADED[name] = true */
  }
  return 1;
}

static const luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {NULL, NULL}
};

LUALIB_API int luaopen_package (lua_State *L) {
  /* set field `loaded' */
  luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 2);
  lua_pop(L, 1);
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  luaL_register(L, NULL, ll_funcs);  /* open lib into global table */
  return 0;  /* return 'package' table */
}

lua_State* skr_lua_newstate(skr_vfs_t* vfs)
{
    lua_State* L = lua_newstate(
    +[](void* ud, void* ptr, size_t osize, size_t nsize) -> void* {
        if (nsize == 0)
        {
            sakura_free(ptr);
            return nullptr;
        }
        else if (osize == 0)
        {
            return sakura_malloc(nsize);
        }
        else
        {
            return sakura_realloc(ptr, nsize);
        }
    },
    nullptr);
    auto extra = SkrNew<skr_lua_state_extra_t>();
    extra->vfs = vfs;
    *(void**)lua_getextraspace(L) = extra;

    // open standard libraries
    luaL_openlibs(L);
    luaopen_package(L);
    // // insert loader
    // lua_pushcfunction(L,skr_load_file,"skr_load_file");
    // int loaderFunc = lua_gettop(L);

    // lua_getglobal(L,"package");
    // lua_getfield(L,-1,"searchers");

    // int loaderTable = lua_gettop(L);

    // for(auto i = lua_objlen(L,loaderTable) + 1; i > 2u; i--) 
    // {
    //     lua_rawgeti(L,loaderTable,i-1);
    //     lua_rawseti(L,loaderTable,i);
    // }
    // lua_pushvalue(L,loaderFunc);
    // lua_rawseti(L,loaderTable,2);
    // lua_settop(L, 0);

    // create skr global table
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "skr");
    lua_pop(L, 1);

    // bind utilities
    lua_pushcfunction(L, +[](lua_State* L) -> int {
        auto asize = luaL_checkinteger(L, 1);
        auto nsize = luaL_checkinteger(L, 2);
        lua_createtable(L, (int)asize, (int)nsize);
        return 1;
    }, "newtable");
    lua_setglobal(L, "newtable");

    // bind skr types
    skr::lua::bind_unknown(L);
    skr::lua::bind_skr_guid(L);
    skr::lua::bind_skr_resource_handle(L);
    skr::lua::bind_ecs(L);

    // bind skr functions
    skr::lua::bind_skr_log(L);


    return L;
}

void skr_lua_close(lua_State* L)
{
    lua_close(L);
    ExtraSpace.erase(L);
}

namespace skr::lua
{
void dtor_unique(void* p)
{
    ((skr::lua::destructor_t)((char*)p + sizeof(void*)))(*(void**)p);
}
void dtor_shared(void* p)
{
    ((skr::SPtr<void>*)((char*)p + sizeof(void*)))->reset();
}
void dtor_object(void* p)
{
    ((skr::SObjectPtr<skr::SInterface>*)((char*)p + sizeof(void*)))->reset();
}
void bind_unknown(lua_State* L)
{
    luaL_Reg metamethods[] = {
        { "__eq", [](lua_State* L) -> int {
            void* a = *(void**)lua_touserdata(L, 1);
            void* b = *(void**)lua_touserdata(L, 2);
            lua_pushboolean(L, a == b);
            return 1;
        } },
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "skr_opaque_t");
    luaL_register(L, nullptr, metamethods);
    lua_pop(L, 1);
    luaL_Reg uniquemetamethods[] = {
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[unique]skr_opaque_t");
    luaL_register(L, nullptr, uniquemetamethods);
    lua_pop(L, 1);
    luaL_Reg sharedmetamethods[] = {
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[shared]skr_opaque_t");
    luaL_register(L, nullptr, sharedmetamethods);
    lua_pop(L, 1);
    luaL_Reg objectmetamethods[] = {
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[object]skr_opaque_t");
    luaL_register(L, nullptr, objectmetamethods);
    lua_pop(L, 1);
}

void skr_lua_close(lua_State* L)
{
    auto extra = (skr_lua_state_extra_t*)*(void**)lua_getextraspace(L);
    SKR_ASSERT(extra != nullptr);
    SkrDelete(extra);
    lua_close(L);
}

void bind_skr_guid(lua_State* L)
{
    luaL_Reg metamethods[] = {
        { "__tostring", +[](lua_State* L) -> int {
            auto guid = (skr_guid_t*)luaL_checkudata(L, 1, "skr_guid_t");
            lua_pushstring(L, skr::format(u8"{}", *guid).c_str());
            return 1;
        } },
        { "__eq", +[](lua_State* L) -> int {
            auto guid1 = (skr_guid_t*)luaL_checkudata(L, 1, "skr_guid_t");
            auto guid2 = (skr_guid_t*)luaL_checkudata(L, 2, "skr_guid_t");
            lua_pushboolean(L, *guid1 == *guid2);
            return 1;
        } },
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "skr_guid_t");
    luaL_register(L, nullptr, metamethods);
    lua_pop(L, 1);
}

void *luaL_testudata (lua_State *L, int i, const char *tname) {
  void *p = lua_touserdata(L, i);
  luaL_checkstack(L, 2, "not enough stack slots");
  if (p == NULL || !lua_getmetatable(L, i))
    return NULL;
  else {
    int res = 0;
    luaL_getmetatable(L, tname);
    res = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);
    if (!res)
      p = NULL;
  }
  return p;
}

void luaL_setmetatable (lua_State *L, const char *tname) {
  luaL_checkstack(L, 1, "not enough stack slots");
  luaL_getmetatable(L, tname);
  lua_setmetatable(L, -2);
}

void dtor_resource_handle(void* p) {
    auto resource = (skr_resource_handle_t*)p;
    if (resource->is_resolved())
        resource->unload();
}

void bind_skr_resource_handle(lua_State* L)
{
    lua_getglobal(L, "skr");
    // resource constructor
    lua_pushcfunction(
    L, +[](lua_State* L) -> int {
        if (luaL_testudata(L, 1, "skr_guid_t"))
        {
            const skr_guid_t* guid = skr::lua::check_guid(L, 1);
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdatadtor(L, sizeof(skr_resource_handle_t), dtor_resource_handle);
            new (resource) skr_resource_handle_t(*guid);
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else if (lua_isstring(L, 1))
        {
            auto str = (const char8_t*)lua_tostring(L, 1);
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdatadtor(L, sizeof(skr_resource_handle_t), dtor_resource_handle);
            new (resource) skr_resource_handle_t(skr::guid::make_guid_unsafe(str));
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else
        {
            luaL_error(L, "invalid arguments for skr_resource_handle_t constructor");
            return 0;
        }
    }, "resource_handle");
    lua_setfield(L, -2, "resource_handle");
    lua_pop(L, 1);

    luaL_newmetatable(L, "skr_resource_handle_t");

    luaL_Reg metamethods[] = {
        { "__tostring", +[](lua_State* L) -> int {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            lua_pushstring(L, skr::format(u8"resource {}", resource->get_serialized()).c_str());
            return 1;
        } },
        { "__eq", +[](lua_State* L) -> int {
            auto resource1 = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            auto resource2 = (skr_resource_handle_t*)luaL_checkudata(L, 2, "skr_resource_handle_t");
            lua_pushboolean(L, resource1->get_serialized() == resource2->get_serialized());
            return 1;
        } },
        { "__index", +[](lua_State* L) -> int {
            auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
            (void)resource;
            const char* key = luaL_checkstring(L, 2);
            switchname(key)
            {
                casestr("resolve")
                {
                    lua_pushcfunction(
                    L, +[](lua_State* L) -> int {
                        auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
                        bool requireInstall = lua_toboolean(L, 2);
                        if (!resource->is_resolved())
                            resource->resolve(requireInstall, (uint64_t)L, SKR_REQUESTER_SCRIPT);
                        return 0;
                    }, "resolve");
                    return 1;
                }
                casestr("is_resolved")
                {
                    lua_pushcfunction(
                    L, +[](lua_State* L) -> int {
                        auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
                        lua_pushboolean(L, resource->is_resolved());
                        return 1;
                    }, "is_resolved");
                    return 1;
                }
                casestr("get_resolved")
                {
                    lua_pushcfunction(
                    L, +[](lua_State* L) -> int {
                        auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
                        if (!resource->is_resolved())
                        {
                            return 0;
                        }
                        auto ptr = resource->get_resolved();
                        if (!ptr)
                        {
                            return 0;
                        }
                        auto tid = resource->get_type();
                        auto type = skr_get_type(&tid);
                        lua_pushlightuserdata(L, ptr);
                        luaL_getmetatable(L, (const char*)type->Name());
                        lua_setmetatable(L, -2);
                        return 1;
                    }, "get_resolved");
                    return 1;
                }
                casestr("unload")
                {
                    lua_pushcfunction(
                    L, +[](lua_State* L) -> int {
                        auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
                        if (resource->is_resolved())
                            resource->unload();
                        else
                            SKR_LOG_DEBUG("skr_resource_handle_t::unload called on unresolved resource.");
                        return 0;
                    }, "unload");
                    return 1;
                }
                default: {
                    luaL_error(L, "skr_resource_handle_t does not have a member named '%s'", key);
                    return 0;
                }
            }
            SKR_UNREACHABLE_CODE()
            return 0;
        } },
        { nullptr, nullptr }
    };
    luaL_register(L, nullptr, metamethods);
    lua_pop(L, 1);
}


inline void split(const eastl::u8string_view& s, eastl::vector<eastl::u8string_view>& tokens, const eastl::u8string_view& delimiters = u8" ")
{
    using namespace::skr;
    eastl::u8string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    eastl::u8string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (eastl::u8string::npos != pos || eastl::u8string::npos != lastPos)
    {
        auto substr = s.substr(lastPos, pos - lastPos);
        tokens.push_back(substr); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

inline eastl::u8string join(const eastl::vector<eastl::u8string_view>& tokens, const eastl::u8string_view& delimiters = u8" ")
{
    eastl::u8string s;
    for (auto& token : tokens)
    {
        if (!s.empty())
            s.insert(s.end(), delimiters.begin(), delimiters.end());
        s.insert(s.end(), token.begin(), token.end());
    }
    return s;
}

template<int level>
int skr_lua_log(lua_State* L)
{
    lua_Debug ar;
    lua_getinfo(L, 1, "nSl", &ar);
    auto str = skr::format(u8"[{} : {}]:\t", (uint64_t)ar.what, (const char8_t*)ar.name);
    int top = lua_gettop(L);
    for(int n=1;n<=top;n++) {
        size_t len;
        const char8_t* s = (const char8_t*)luaL_tolstring(L, n, &len);
        str += u8"\t";
        //TODO: use string builder?
        if(s) str += s;
    }
    const int line = ar.currentline;
    auto src = (const char8_t*)ar.source;
    if(line != -1)
    {
        eastl::u8string_view Source(src);
        if (Source.ends_with(u8".lua"))
            Source = Source.substr(0, Source.size() - 4);
        if (Source.starts_with(u8"@"))
            Source = Source.substr(1);
        eastl::vector<eastl::u8string_view> tokens;
        split(Source, tokens, u8"/");
        
        const auto modulename = join(tokens, u8".");
        log_log(level, (const char*)modulename.c_str(), line, str.c_str());
    }
    else 
    {
        log_log(level, "unknown", 0, str.c_str());
    }
    return 0;
}

void bind_skr_log(lua_State* L)
{
    //lua_atpanic(L, +[](lua_State* L) -> int {
    //    SKR_LOG_FATAL("Lua panic: %s", lua_tostring(L, -1));
    //    return 0;
    //});
    lua_getglobal(L, "skr");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_INFO>, "print");
    lua_setfield(L, -2, "print");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_DEBUG>, "log_debug");
    lua_setfield(L, -2, "log_debug");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_INFO>, "log_info");
    lua_setfield(L, -2, "log_info");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_WARN>, "log_warn");
    lua_setfield(L, -2, "log_warn");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_ERROR>, "log_error");
    lua_setfield(L, -2, "log_error");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_FATAL>, "log_fatal");
    lua_setfield(L, -2, "log_fatal");
    lua_pop(L, 1);
}
} // namespace skr::lua

namespace skr::lua
{
int push_guid(lua_State* L, const skr_guid_t* guid)
{
    auto ud = (skr_guid_t*)lua_newuserdata(L, sizeof(skr_guid_t));
    *ud = *guid;
    luaL_getmetatable(L, "skr_guid_t");
    lua_setmetatable(L, -2);
    return 1;
}

const skr_guid_t* check_guid(lua_State* L, int index)
{
    return (skr_guid_t*)luaL_checkudata(L, index, "skr_guid_t");
}

const skr_guid_t* opt_guid(lua_State* L, int index, const skr_guid_t* def)
{
    if (lua_isnoneornil(L, index))
        return def;
    return check_guid(L, index);
}

int push_enum(lua_State *L, long long v)
{
    lua_pushinteger(L, v);
    return 1;
}

long long check_enum(lua_State *L, int index)
{
    return luaL_checkinteger(L, index);
}

long long opt_enum(lua_State *L, int index, long long def)
{
    return luaL_optinteger(L, index, def);
}

int push_string(lua_State* L, const skr::string& str)
{
    lua_pushstring(L, str.c_str());
    return 1;
}

int push_string(lua_State* L, eastl::u8string_view str)
{
    lua_pushlstring(L, (const char*)str.data(), str.size());
    return 1;
}

skr::string check_string(lua_State* L, int index)
{
    return (const char8_t*)lua_tostring(L, index);
}

skr::string opt_string(lua_State* L, int index, const skr::string& def)
{
    return { (const char8_t*)luaL_optstring(L, index, def.c_str()) };
}

int push_resource(lua_State* L, const skr_resource_handle_t* resource)
{
    auto ud = (skr_resource_handle_t*)lua_newuserdatadtor(L, sizeof(skr_resource_handle_t), dtor_resource_handle);
    new (ud) skr_resource_handle_t(*resource, (uint64_t)L, SKR_REQUESTER_SCRIPT);
    luaL_getmetatable(L, "skr_resource_handle_t");
    lua_setmetatable(L, -2);
    return 1;
}

const skr_resource_handle_t* check_resource(lua_State* L, int index)
{
    return (skr_resource_handle_t*)luaL_checkudata(L, index, "skr_resource_handle_t");
}

const skr_resource_handle_t* opt_resource(lua_State* L, int index, const skr_resource_handle_t* def)
{
    if (lua_isnoneornil(L, index))
        return def;
    return check_resource(L, index);
}

int push_unknown(lua_State *L, void *value, std::string_view tid)
{
    *(void**)lua_newuserdata(L, sizeof(void*)) = value;
    luaL_getmetatable(L, tid.data());
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        luaL_getmetatable(L, "skr_opaque_t");
        lua_setmetatable(L, -2);
        return 0;
    }
    else 
    {
        lua_setmetatable(L, -2);
        return 1;
    }
}

void* skr_check_unknown(lua_State *L, int index, std::string_view tid, std::string_view unknown)
{
    void *p = lua_touserdata(L, index);
    if (p != NULL) {  /* value is a userdata? */
        if (lua_getmetatable(L, index)) {  /* does it have a metatable? */
            luaL_getmetatable(L, tid.data());  /* get correct metatable */
            if (!lua_rawequal(L, -1, -2))  /* not the same? */
            {
                if(!unknown.empty()) /* check again */
                {
                    luaL_getmetatable(L, unknown.data());
                    if (!lua_rawequal(L, -1, -3))  /* not the same? */
                        p = NULL;  /* value is a userdata with wrong metatable */
                    lua_pop(L, 1);
                }
                else if(!lua_isnil(L, -1))
                {
                    p = NULL;
                }
            }
            lua_pop(L, 2);  /* remove both metatables */
        }
    }
    luaL_argexpected(L, p != 0, index, tid.data());
    return p;
}

void* check_unknown(lua_State *L, int index, std::string_view tid)
{
    return *(void**)skr_check_unknown(L, index, tid, "");
} 

int push_unknown_value(lua_State *L, const void *value, std::string_view tid, size_t size, copy_constructor_t copy_constructor, destructor_t destructor)
{
    void *p = lua_newuserdatadtor(L, sizeof(void*) * 2+ size, dtor_unique);
    void *obj = (char*)p + sizeof(void*) * 2;
    copy_constructor(obj, value);
    *(void**)p = obj;
    *(destructor_t*)((char*)p + sizeof(void*)) = destructor;
    luaL_getmetatable(L, tid.data());
    if (lua_isnil(L, -1))
    {
        SKR_UNREACHABLE_CODE();
        lua_pop(L, 1);
        luaL_getmetatable(L, "[unique]skr_opaque_t");
        lua_setmetatable(L, -2);
        return 0;
    }
    else 
    {
        lua_setmetatable(L, -2);
        return 1;
    }
}

int push_sptr(lua_State *L, const skr::SPtr<void> &value, std::string_view tid)
{
    void* p = lua_newuserdatadtor(L, sizeof(void*) + sizeof(skr::SPtr<void>), dtor_shared);
    void* obj = (char*)p + sizeof(void*);
    auto ptr = new (obj) skr::SPtr<void>(value);
    *(void**)p = ptr->get();
    luaL_getmetatable(L, tid.data());
    if (lua_isnil(L, -1))
    {
        SKR_UNREACHABLE_CODE();
        lua_pop(L, 1);
        luaL_getmetatable(L, "[shared]skr_opaque_t");
        lua_setmetatable(L, -2);
        return 0;
    }
    else 
    {
        lua_setmetatable(L, -2);
        return 1;
    }
}

skr::SPtr<void> check_sptr(lua_State *L, int index, std::string_view tid)
{
    void*  p = skr_check_unknown(L, index, tid, "[shared]skr_opaque_t");
    return *(skr::SPtr<void>*)((char*)p + sizeof(void*));
}

int push_sobjectptr(lua_State *L, const skr::SObjectPtr<SInterface> &value, std::string_view tid)
{
    void* p = lua_newuserdatadtor(L, sizeof(void*) + sizeof(skr::SObjectPtr<SInterface>), dtor_object);
    void* obj = (char*)p + sizeof(void*);
    auto ptr = new (obj) skr::SObjectPtr<SInterface>(value);
    *(void**)p = ptr->get();
    luaL_getmetatable(L, tid.data());
    if (lua_isnil(L, -1))
    {
        SKR_UNREACHABLE_CODE();
        lua_pop(L, 1);
        luaL_getmetatable(L, "[object]skr_opaque_t");
        lua_setmetatable(L, -2);
        return 0;
    }
    else 
    {
        lua_setmetatable(L, -2);
        return 1;
    }
}

skr::SObjectPtr<SInterface> check_sobjectptr(lua_State *L, int index, std::string_view tid)
{
    void*  p = skr_check_unknown(L, index, tid, "[object]skr_opaque_t");
    return *(skr::SObjectPtr<SInterface>*)((char*)p + sizeof(void*));

}

}// namespace skr::lua