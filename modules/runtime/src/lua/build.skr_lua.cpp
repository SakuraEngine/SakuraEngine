#include "lua/skr_lua.h"
#include "platform/memory.h"
#include "utils/defer.hpp"
#include "utils/types.h"
#include "utils/format.hpp"
#include "platform/guid.hpp"
#include "resource/resource_handle.h"
#include "lua/bind.hpp"
#include "utils/log.h"
#include "platform/vfs.h"
#include "ecs/dual.h"

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
RUNTIME_EXTERN_C RUNTIME_API void skr_lua_setroot(lua_State* L, const char* directory);
RUNTIME_EXTERN_C int
luaopen_clonefunc(lua_State *L);

void replaceAll(skr::string& str, const skr::string_view& from, const skr::string_view& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = skr::string_view(str).find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to.data(), to.length());
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

int skr_load_file(lua_State* L) {
    skr_lua_state_extra_t* extra = (skr_lua_state_extra_t*)*(void**)lua_getextraspace(L);
    const char* fn = luaL_checkstring(L, 1);
    skr::string path = fn;
    replaceAll(path, ".", "/");
    skr::string_view exts[] = { ".lua", ".luac" };
    skr_vfile_t* file = nullptr;
    for(int i=0; i<2; ++i)
    {
        skr::string fullpath = path + exts[i].data();
        file = skr_vfs_fopen(extra->vfs, fullpath.c_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        if(file)
            break;
    }
    if(!file)
    {
        SKR_LOG_ERROR("[lua] Failed to open file: %s", fn);
        return 0;
    }
    SKR_DEFER({ skr_vfs_fclose(file); });
    auto size = skr_vfs_fsize(file);
    eastl::vector<char> buffer(size);
    skr_vfs_fread(file, buffer.data(), 0, size);
    skr::string name = skr::string("@") + fn;
    if(luaL_loadbuffer(L,buffer.data(), size, name.c_str())==0) {
        return 1;
    }
    else {
        const char* err = lua_tostring(L,-1);
        SKR_LOG_ERROR("[lua] Failed to load file: %s", err);
        lua_pop(L,1);
    }
    return 0;
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

    // insert loader
    lua_pushcfunction(L,skr_load_file);
    int loaderFunc = lua_gettop(L);

    lua_getglobal(L,"package");
    lua_getfield(L,-1,"searchers");

    int loaderTable = lua_gettop(L);

    for(int i=lua_rawlen(L,loaderTable)+1;i>2;i--) {
        lua_rawgeti(L,loaderTable,i-1);
        lua_rawseti(L,loaderTable,i);
    }
    lua_pushvalue(L,loaderFunc);
    lua_rawseti(L,loaderTable,2);
    lua_settop(L, 0);

    // create skr global table
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "skr");
    lua_pop(L, 1);

    // bind clone
    lua_getglobal(L, "skr");
    luaopen_clonefunc(L);
    lua_setfield(L, -2, "clonefunc");
    lua_pop(L, 1);

    // bind skr types
    skr::lua::bind_unknown(L);
    skr::lua::bind_skr_guid(L);
    skr::lua::bind_skr_resource_handle(L);
    skr::lua::bind_ecs(L);

    // bind skr functions
    skr::lua::bind_skr_log(L);

    return L;
}

namespace skr::lua
{
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
    luaL_setfuncs(L, metamethods, 0);
    lua_pop(L, 1);
    luaL_Reg uniquemetamethods[] = {
        { "_gc", [](lua_State* L) -> int {
            void* p = lua_touserdata(L, 1);
            ((skr::lua::destructor_t)((char*)p + sizeof(void*)))(*(void**)p);
            return 0;
        }},
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[unique]skr_opaque_t");
    luaL_setfuncs(L, uniquemetamethods, 0);
    lua_pop(L, 1);
    luaL_Reg sharedmetamethods[] = {
        { "_gc", [](lua_State* L) -> int {
            void* p = lua_touserdata(L, 1);
            ((skr::SPtr<void>*)((char*)p + sizeof(void*)))->reset();
            return 0;
        }},
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[shared]skr_opaque_t");
    luaL_setfuncs(L, sharedmetamethods, 0);
    lua_pop(L, 1);
    luaL_Reg objectmetamethods[] = {
        { "_gc", [](lua_State* L) -> int {
            void* p = lua_touserdata(L, 1);
            ((skr::SObjectPtr<skr::SInterface>*)((char*)p + sizeof(void*)))->reset();
            return 0;
        }},
        metamethods[0],
        metamethods[1],
        { nullptr, nullptr }
    };
    luaL_newmetatable(L, "[object]skr_opaque_t");
    luaL_setfuncs(L, objectmetamethods, 0);
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
             lua_pushstring(L, skr::format("{}", *guid).c_str());
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
    luaL_setfuncs(L, metamethods, 0);
    lua_pop(L, 1);
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
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
            new (resource) skr_resource_handle_t(*guid);
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else if (lua_isstring(L, 1))
        {
            const char* str = lua_tostring(L, 1);
            skr_resource_handle_t* resource = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
            new (resource) skr_resource_handle_t(skr::guid::make_guid_unsafe(skr::string_view{ str }));
            luaL_setmetatable(L, "skr_resource_handle_t");
            return 1;
        }
        else
        {
            return luaL_error(L, "invalid arguments for skr_resource_handle_t constructor");
        }
    });
    lua_setfield(L, -2, "resource_handle");
    lua_pop(L, 1);

    luaL_newmetatable(L, "skr_resource_handle_t");

    luaL_Reg metamethods[] = {
        { "__gc", +[](lua_State* L) -> int {
             auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
             if (resource->is_resolved())
                 resource->unload();
             return 0;
         } },
        { "__tostring", +[](lua_State* L) -> int {
             auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
             lua_pushstring(L, skr::format("resource {}", resource->get_serialized()).c_str());
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
                     });
                     return 1;
                 }
                 casestr("is_resolved")
                 {
                     lua_pushcfunction(
                     L, +[](lua_State* L) -> int {
                         auto resource = (skr_resource_handle_t*)luaL_checkudata(L, 1, "skr_resource_handle_t");
                         lua_pushboolean(L, resource->is_resolved());
                         return 1;
                     });
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
                         luaL_getmetatable(L, type->Name());
                         lua_setmetatable(L, -2);
                         return 1;
                     });
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
                     });
                     return 1;
                 }
                 default: {
                     return luaL_error(L, "skr_resource_handle_t does not have a member named '%s'", key);
                 }
             }
             SKR_UNREACHABLE_CODE()
             return 0;
         } },
        { nullptr, nullptr }
    };
    luaL_setfuncs(L, metamethods, 0);
    lua_pop(L, 1);
}


inline void split(const skr::string_view& s, eastl::vector<skr::string_view>& tokens, const skr::string_view& delimiters = " ")
{
    using namespace::skr;
    string::size_type lastPos = s.find_first_not_of(delimiters, 0);
    string::size_type pos = s.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos)
    {
        auto substr = s.substr(lastPos, pos - lastPos);
        tokens.push_back(substr); // use emplace_back after C++11
        lastPos = s.find_first_not_of(delimiters, pos);
        pos = s.find_first_of(delimiters, lastPos);
    }
}

inline skr::string join(const eastl::vector<skr::string_view>& tokens, const skr::string_view& delimiters = " ")
{
    using namespace::skr;
    string s;
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
    lua_getstack(L, 1, &ar);
    lua_getinfo(L, "nSl", &ar);
    skr::string str = skr::format("[{} : {}]:\t", ar.namewhat, ar.name);
    int top = lua_gettop(L);
    for(int n=1;n<=top;n++) {
        size_t len;
        const char* s = luaL_tolstring(L, n, &len);
        str+="\t";
        //TODO: use string builder?
        if(s) str+=s;
    }
    const int line = ar.currentline;
    const char* src = ar.source;
    if(line != -1)
    {
        skr::string_view Source(src);
        if (Source.ends_with(".lua"))
            Source = Source.substr(0, Source.size() - 4);
        if (Source.starts_with("@"))
            Source = Source.substr(1);
        eastl::vector<skr::string_view> tokens;
        split(Source, tokens, "/");
        
        const auto modulename = join(tokens, ".");
        log_log(level, modulename.c_str(), line, str.c_str());
    }
    else 
    {
        log_log(level, "unknown", 0, str.c_str());
    }
    return 0;
}

void bind_skr_log(lua_State* L)
{
    lua_atpanic(L, +[](lua_State* L) -> int {
        SKR_LOG_FATAL("Lua panic: %s", lua_tostring(L, -1));
        return 0;
    });
    lua_getglobal(L, "skr");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_INFO>);
    lua_setfield(L, -2, "print");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_DEBUG>);
    lua_setfield(L, -2, "log_debug");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_INFO>);
    lua_setfield(L, -2, "log_info");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_WARN>);
    lua_setfield(L, -2, "log_warn");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_ERROR>);
    lua_setfield(L, -2, "log_error");
    lua_pushcfunction(L, &skr_lua_log<SKR_LOG_LEVEL_FATAL>);
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

int push_string(lua_State* L, skr::string_view str)
{
    lua_pushlstring(L, str.data(), str.size());
    return 1;
}

skr::string check_string(lua_State* L, int index)
{
    return lua_tostring(L, index);
}

skr::string opt_string(lua_State* L, int index, const skr::string& def)
{
    return luaL_optstring(L, index, def.c_str());
}

int push_resource(lua_State* L, const skr_resource_handle_t* resource)
{
    auto ud = (skr_resource_handle_t*)lua_newuserdata(L, sizeof(skr_resource_handle_t));
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
    void *p = lua_newuserdata(L, sizeof(void*) * 2+ size);
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
    void* p = lua_newuserdata(L, sizeof(void*) + sizeof(skr::SPtr<void>));
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
    void* p = lua_newuserdata(L, sizeof(void*) + sizeof(skr::SObjectPtr<SInterface>));
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

// from https://github.com/cloudwu/luareload/blob/proto/clonefunc.c
extern "C"
{
#include <lua/lstate.h>
#include <lua/lobject.h>
#include <lua/lfunc.h>
#include <lua/lgc.h>
}

static int
lclone(lua_State *L) {
	if (!lua_isfunction(L, 1) || lua_iscfunction(L,1))
		return luaL_error(L, "Need lua function");
	const LClosure *c = (const LClosure *)lua_topointer(L,1);
	int n = (int)luaL_optinteger(L, 2, 0);
	if (n < 0 || n > c->p->sizep)
		return 0;
	luaL_checkstack(L, 1, NULL);
	Proto *p;
	if (n==0) {
		p = c->p;
	} else {
		p = c->p->p[n-1];
	}

	lua_lock(L);
	LClosure *cl = luaF_newLclosure(L, p->sizeupvalues);
	luaF_initupvals(L, cl);
	cl->p = p;
	setclLvalue2s(L, L->top++, cl);
	lua_unlock(L);

	return 1;
}

static int
lproto(lua_State *L) {
	if (!lua_isfunction(L, 1) || lua_iscfunction(L,1))
		return 0;
	const LClosure *c = (const LClosure *)lua_topointer(L,1);
	lua_pushlightuserdata(L, c->p);
	lua_pushinteger(L, c->p->sizep);
	return 2;
}

int
luaopen_clonefunc(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "clone", lclone },
		{ "proto", lproto },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}