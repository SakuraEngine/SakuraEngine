// BEGIN LUA GENERATED
#include "lua/bind.hpp"
extern "C"
{
    #include "lua.h"
    #include "lualib.h"
}
#include "misc/hash.h"
<%
    categories = {}
    for enum in generator.filter_types(db.enums):
        finalcat = categories
        if hasattr(enum.attrs.scriptable, "category"):
            category = enum.attrs.scriptable.category
            subcategories = category.split(".")
            for subcategory in subcategories:
                finalcat = finalcat.setdefault(subcategory, {})
        finalcat.setdefault("___ENUMS___", []).append(enum)
    lua_binders = []
    for function in generator.filter_types(db.functions):
        finalcat = categories
        if hasattr(function.attrs, "lua_binder"):
            lua_binders.append(function)
            continue
        if hasattr(function.attrs.scriptable, "category"):
            category = function.attrs.scriptable.category
            subcategories = category.split(".")
            for subcategory in subcategories:
                finalcat = finalcat.setdefault(subcategory, {})
        finalcat.setdefault("___FUNCTIONS___", []).append(function)
%>
<%def name="bind_function(function, record)">
        lua_pushcfunction(L, +[](lua_State* L)
        {
            int currArg = 1; (void)currArg;
            int usedArg = 0; (void)usedArg;
        %if record:
            auto& record = **reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
        %endif
            <% 
                out_params = []
                out_params_count = 0
                has_return = function.retType != "void"
            %>
        %for i, (name, param) in enumerate(vars(function.parameters).items()):
            <%
                name = name if name else "param" + str(i)
                if hasattr(param.attrs, "out"):
                    out_params.append(name)
                    out_params_count += 1
                elif hasattr(param.attrs, "inout"):
                    out_params.append(name)
            %>
        
        %if param.type.replace(" ", "") == "void*":
            void* ${name} = L;
            usedArg = 0;
        %elif param.isCallback:
            <%
                callback = param.functor
                cparamlist = ", ".join([cparam.type + " " + cname for cname, cparam in vars(callback.parameters).items()])
                ccapture = ""
                if param.isFunctor:
                    ccapture = "L"
                chas_return = callback.retType != "void" and callback.retType
            %>
            if(!lua_isfunction(L, ${i + 1 - out_params_count}))
                luaL_error(L, "expected function for parameter ${name}");
            usedArg = 1;
            using ${name}_t = ${param.type};
            ${name}_t ${name} = [${ccapture}](${cparamlist})
            {
                %for name, param in vars(callback.parameters).items():
                %if hasattr(param.attrs, "userdata"):
                lua_State* L = reinterpret_cast<lua_State*>(${name});
                %endif
                %endfor
                int oldTop = lua_gettop(L);
                lua_pushvalue(L, ${i + 1 - out_params_count});
                int ncargs = 0;
                %for name, param in vars(callback.parameters).items():
                %if not hasattr(param.attrs, "userdata") and not hasattr(param.attrs, "out"):
                ncargs += skr::lua::push<${param.type}>(L, ${name});
                %endif
                %endfor
                if(lua_pcall(L, ncargs, LUA_MULTRET, 0) != LUA_OK)
                {
                    lua_getglobal(L, "skr");
                    lua_getfield(L, -1, "log_error");
                    lua_pushvalue(L, -3);
                    lua_call(L, 1, 0);
                    lua_pop(L, 2);
                    %if chas_return:
                    return {};
                    %else:
                    return;
                    %endif
                }
                int currRet = -1; (void)currRet;
                int usedRet = 0; (void)usedRet;
                %for name, param in reversed(vars(callback.parameters).items()):
                %if hasattr(param.attrs, "out") or hasattr(param.attrs, "inout"):
                decltype(auto) _${name} = skr::lua::deref<${param.type}>(${name});
                _${name} = skr::lua::check<decltype(_${name})>(L, currRet, usedRet);
                currRet -= usedRet;
                %endif
                %endfor
                %if chas_return:
                auto result = skr::lua::check<${callback.retType}>(L, currRet, usedRet);
                currRet -= usedRet;
                %endif
                lua_settop(L, oldTop);
                %if chas_return:
                return result;
                %endif
            };
        %else:
        %if hasattr(param.attrs, "out") or hasattr(param.attrs, "inout"):
            using ${name}_t = skr::lua::refed_t<${param.type}>;
        %else:
            using ${name}_t = std::remove_reference_t<${param.type}>;
        %endif
        %if hasattr(param.attrs, "out"):
            ${name}_t _${name};
            ${param.type} ${name} = skr::lua::ref<${param.type}>(_${name});
        %elif hasattr(param.attrs, "inout"):
            ${name}_t _${name} = skr::lua::check<${name}_t>(L, currArg, usedArg);
            ${param.type} ${name} = skr::lua::ref<${param.type}>(_${name});
        %else:
            ${param.type} ${name} = skr::lua::check<${name}_t>(L, currArg, usedArg);
        %endif
        %endif
            currArg += usedArg;
        %endfor
        int nRet = 0;
        %if has_return:
            ${function.retType} result = \
        %endif
        %if record:
            record.${db.short_name(function.name)}\
        %else:
            ${function.name}\
        %endif
            (${", ".join([name for name in vars(function.parameters).keys()])});
        %if has_return:
            nRet += skr::lua::push<${function.retType}>(L, result);
        %endif
        %for name in out_params:
            nRet += skr::lua::push<${name}_t>(L, ${name});
        %endfor
            return nRet;
        }, "${db.short_name(function.name)}");
</%def>
<%def name="bind_category(cat)">
    <% 
        enums = cat.get("___ENUMS___", [])
        functions = cat.get("___FUNCTIONS___", [])
    %>
    %for enum in enums:
    %if enum.isScoped:
    lua_newtable(L);
    %endif
    %for name, enumerator in vars(enum.values).items():
    lua_pushinteger(L, ${enumerator.value});
    lua_setfield(L, -2, "${name}");
    %endfor
    %if enum.isScoped:
    lua_setfield(L, -2, "${db.short_name(enum.name)}");
    %endif
    %endfor
    %for function in functions:
    ${bind_function(function, None)}
    lua_setfield(L, -2, "${db.short_name(function.name)}");
    %endfor
    %for subcat, value in cat.items():
    %if subcat != "___ENUMS___" and subcat != "___FUNCTIONS___":
    lua_newtable(L);
    ${bind_category(value)}
    lua_setfield(L, -2, "${subcat}");
    %endif
    %endfor
</%def>
void skr_lua_open_${module}(lua_State* L)
{
    lua_newtable(L);
    // bind enums

    ${bind_category(categories)}
    // bind records
%for record in generator.filter_types(db.records):
    {
    luaL_Reg basemetamethods[] = {
        {"__index", +[](lua_State* L)
        {
            auto& record = **reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            auto key = lua_tostring(L, 2);
            switchname(key)
            {
            %for name, field in vars(record.fields).items():
                <% if hasattr(field.attrs, "native"): continue %>
                casestr("${name}")
                {
                    return skr::lua::push<${field.type}>(L, record.${name});
                }
            %endfor
            %for method in record.methods:
                <% if hasattr(method.attrs, "native"): continue %>
                casestr("${db.short_name(method.name)}")
                {
                    ${bind_function(method, record)}
                    return 1;
                }
            %endfor
                default:
                    luaL_error(L, "invalid field name : %s", key);
                    return 0;
            }
            SKR_UNREACHABLE_CODE();
            return 0;
        }},
        {"__newindex", +[](lua_State* L)
        {
            auto& record = **reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            auto key = lua_tostring(L, 2);
            int used = 0;
            switchname(key)
            {
            %for name, field in vars(record.fields).items():
                <% if hasattr(field.attrs, "native"): continue %>
                casestr("${name}")
                {
                    record.${name} = skr::lua::check<${field.type}>(L, 3, used);
                    return 0;
                }
            %endfor
                default:
                    luaL_error(L, "invalid field name %s", key);
                    return 0;
            }
            SKR_UNREACHABLE_CODE();
            return 0;
        }},
        {"__eq", +[](lua_State* L)
        {
            auto lhs = *reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            auto rhs = *reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            lua_pushboolean(L, lhs == rhs);
            return 1;
        }},
        {nullptr, nullptr} // sentinel
    };
    luaL_newmetatable(L, "${record.attrs.guid}");
    luaL_register(L, nullptr, basemetamethods);
    lua_pop(L, 1);
    luaL_Reg sharedmetamedhods[] = {
        {"__gc", +[](lua_State* L)
        {
            using sharedT = skr::lua::SharedUserdata<${record.name}>;
            auto& record = *reinterpret_cast<sharedT*>(lua_touserdata(L, 1));
            record.~sharedT();
            return 0;
        }},
        basemetamethods[0],
        basemetamethods[1],
        basemetamethods[2],
        {nullptr, nullptr} // sentinel
    };
    luaL_newmetatable(L, "[shared]${record.attrs.guid}");
    luaL_register(L, nullptr, sharedmetamedhods);
    lua_pop(L, 1);
    luaL_Reg uniquemetamedhods[] = {
        {"__gc", +[](lua_State* L)
        {
            using T = ${record.name};
            auto& record = **reinterpret_cast<T**>(lua_touserdata(L, 1));
            record.~T();
            return 0;
        }},
        basemetamethods[0],
        basemetamethods[1],
        basemetamethods[2],
        {nullptr, nullptr} // sentinel
    };
    luaL_newmetatable(L, "[unique]${record.attrs.guid}");
    luaL_register(L, nullptr, uniquemetamedhods);
    lua_pop(L, 1);
    }
%endfor
%for function in lua_binders:
    ${function.name}(L);
%endfor 

    lua_setglobal(L, "${module}");
}
// END LUA GENERATED