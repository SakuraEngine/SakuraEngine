

// BEGIN TRAIT GENERATED

%for record in generator.filter_types(db.records):
%if hasattr(record, "namespace"):
namespace ${record.namespace}
{
%endif
    
struct __VTABLE_${record.short_name}
{
<%
    methods = [method for method in record.methods if not method.isStatic]
%>
%for method in methods:
    <%
        params_expr = ", ".join("{} {}".format(parameter.type, name) for name, parameter in vars(method.parameters).items())
        if(params_expr != ""):
            params_expr = ", " + params_expr
        isConst = "const" if method.isConst else ""
    %>
    ${method.retType} (*${db.short_name(method.name)})(${isConst}  void* self ${params_expr});
%endfor
};
template<class T>
struct __VTABLE_${record.short_name}_HELPER
{
    using Self = __VTABLE_${record.short_name}_HELPER<T>;
%for method in methods:
    <%
        params_expr = ", ".join("{} {}".format(parameter.type, name) for name, parameter in vars(method.parameters).items())
        if(params_expr != ""):
            params_expr = ", " + params_expr
        args_expr = ", ".join(name for name, parameter in vars(method.parameters).items())
        isConst = "const" if method.isConst else ""
        isNoexcept = "noexcept" if method.isNothrow else ""
    %>
    static ${method.retType} static_${db.short_name(method.name)}(${isConst} void* self ${params_expr}) ${isNoexcept}
    {
        auto memberAvailable = SKR_VALIDATOR((auto obj, auto... args), obj->${db.short_name(method.name)}(args...));
        if constexpr(memberAvailable(static_cast<${isConst} T*>(0), ${args_expr}))
            return static_cast<${isConst} T*>(self)->${db.short_name(method.name)}(${args_expr});
        else
            return ${db.short_name(method.name)}(static_cast<${isConst} T*>(self) ${", " + args_expr if args_expr != "" else ""});
    }
%endfor
    static constexpr __VTABLE_${record.short_name} vtable = {
%for method in methods:
        &Self::static_${db.short_name(method.name)},
%endfor
    };
};

%if hasattr(record, "namespace"):
}
%endif

%endfor

// END TRAIT GENERATED