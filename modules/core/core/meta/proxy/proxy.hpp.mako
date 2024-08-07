// BEGIN PROXY GENERATED
%for record in records:
<%record_proxy_data=record.generator_data["proxy"]%>\
// vtable for ${record.short_name}
%if record.namespace:
namespace ${record.namespace}
{
%endif
struct ${record.short_name}_VTable {
%for method in record_proxy_data.methods:
<%method_proxy_data=method.generator_data["proxy"]%>\
${method.ret_type} (*${method.short_name})(${method.dump_const()} void* self ${method.dump_params_with_comma()}) = nullptr;
%endfor
};
template <typename T>
struct ${record.short_name}_VTableTraits {
%for method in record_proxy_data.methods:
<%method_proxy_data=method.generator_data["proxy"]%>\
inline static ${method.ret_type} static_${method.short_name}(${method.dump_const()} void* self ${method.dump_params_with_comma()}) ${method.dump_noexcept()}
{
    [[maybe_unused]] auto validate_method = SKR_VALIDATOR((auto obj, auto... args), obj->${method.short_name}(args...));
    [[maybe_unused]] auto validate_static_method = SKR_VALIDATOR((auto... args), ${method.short_name}(static_cast<${method.dump_const()} T*>(0), args...));

%if method_proxy_data.setter:
    static_assert(std::is_same_v<${method.ret_type}, void>, "Setter must return void");
    if constexpr(validate_method(static_cast<${method.dump_const()} T*>(0) ${method.dump_params_name_only_with_comma()}))
    {
        static_cast<${method.dump_const()} T*>(self)->${method.short_name}(${method.dump_params_name_only()});
    }
    else if constexpr(validate_static_method(${method.dump_params_name_only()}))
    {
        ${method.short_name}(static_cast<${method.dump_const()} T*>(self) ${method.dump_params_name_only_with_comma()});
    }
    else
    {
        static_cast<${method.dump_const()} T*>(self)->${method_proxy_data.setter} = ${method.dump_params_name_only()};
    }
%elif method_proxy_data.getter:
    if constexpr(validate_method(static_cast<${method.dump_const()} T*>(0) ${method.dump_params_name_only_with_comma()}))
    {
        return static_cast<${method.dump_const()} T*>(self)->${method.short_name}(${method.dump_params_name_only()});
    }
    else if constexpr(validate_static_method(${method.dump_params_name_only()}))
    {
        return ${method.short_name}(static_cast<${method.dump_const()} T*>(self) ${method.dump_params_name_only_with_comma()});
    }
    else
    {
        return static_cast<${method.dump_const()} T*>(self)->${method_proxy_data.getter};
    }
%else:
    if constexpr(validate_method(static_cast<${method.dump_const()} T*>(0) ${method.dump_params_name_only_with_comma()}))
    {
        return static_cast<${method.dump_const()} T*>(self)->${method.short_name}(${method.dump_params_name_only()});
    }
    else
    {
        return ${method.short_name}(static_cast<${method.dump_const()} T*>(self) ${method.dump_params_name_only_with_comma()});
    }
    
%endif
}
%endfor
static ${record.short_name}_VTable* get_vtable() {
    static ${record.short_name}_VTable _vtable = {
%for method in record_proxy_data.methods:
        &static_${method.short_name},
%endfor
    };
    return &_vtable;
}
};
%if record.namespace:
}
%endif
%endfor
// END PROXY GENERATED
