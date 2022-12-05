// BEGIN LUA GENERATED
#include "lua/bind.hpp"
#include "lua/lua.hpp"
#include "utils/hash.h"
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

    for function in generator.filter_types(db.functions):
        finalcat = categories
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
        %if record:
            auto& record = **reinterpret_cast<${record.name}**>(lua_touserdata(L, lua_upvalueindex(1)));
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
        %if hasattr(param.attrs, "out") or hasattr(param.attrs, "inout"):
            using ${name}_t = skr::lua::refed_t<${param.type}>;
        %else:
            using ${name}_t = ${param.type};
        %endif
        %if hasattr(param.attrs, "out"):
            ${name}_t _${name};
            ${param.type} ${name} = skr::lua::ref<${param.type}>(_${name});
        %elif hasattr(param.attrs, "inout"):
            ${name}_t _${name} = skr::lua::check<${name}_t>(L, ${i + 1 - out_params_count});
            ${param.type} ${name} = skr::lua::ref<${param.type}>(_${name});
        %else:
            ${param.type} ${name} = skr::lua::check<${name}_t>(L, ${i + 1 - out_params_count});
        %endif
        %endfor
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
            skr::lua::push<${function.retType}>(L, result);
        %endif
        %for name in out_params:
            skr::lua::push<${name}_t>(L, ${name});
        %endfor
            return ${len(out_params) + (1 if has_return else 0)};
        });
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
    lua_getglobal(L, "skr");
    lua_newtable(L);
    // bind enums

    ${bind_category(categories)}
    // bind records
%for record in generator.filter_types(db.records):
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
                    lua_pushlightuserdata(L, &record);
                    lua_setupvalue(L, -2, 1);
                    return 1;
                }
            %endfor
                default:
                    return luaL_error(L, "invalid field name : %s", key);
            }
            SKR_UNREACHABLE_CODE();
            return 0;
        }},
        {"__newindex", +[](lua_State* L)
        {
            auto& record = **reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            auto key = lua_tostring(L, 2);
            switchname(key)
            {
            %for name, field in vars(record.fields).items():
                <% if hasattr(field.attrs, "native"): continue %>
                casestr("${name}")
                {
                    record.${name} = skr::lua::check<${field.type}>(L, 3);
                    return 0;
                }
            %endfor
                default:
                    return luaL_error(L, "invalid field name %s", key);
            }
            SKR_UNREACHABLE_CODE();
            return 0;
        }},
        {"__eq", +[](lua_State* L)
        {
            auto lhs = *reinterpret_cast<${record.name}**>(lua_touserdata(L, 1));
            auto rhs = *reinterpret_cast<${record.name}**>(lua_touserdata(L, 2));
            lua_pushboolean(L, lhs == rhs);
            return 1;
        }},
        {nullptr, nullptr} // sentinel
    };
    luaL_newmetatable(L, "${record.attrs.guid}");
    luaL_setfuncs(L, basemetamethods, 0);
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
    luaL_setfuncs(L, sharedmetamedhods, 0);
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
    luaL_setfuncs(L, uniquemetamedhods, 0);
    lua_pop(L, 1);
%endfor
    lua_setfield(L, -2, "${module}");
}
// END LUA GENERATED