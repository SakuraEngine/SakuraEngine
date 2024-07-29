// BEGIN PROXY GENERATED
%for record in records:
<%record_proxy_data=record.generator_data["proxy"]%>\
%for method in record_proxy_data.methods:
<%method_proxy_data=method.generator_data["proxy"]%>\
${method.ret_type} ${record.name}::${method.short_name}(${method.dump_params()}) ${method.dump_const()} ${method.dump_noexcept()}
{
    return vtable->${method.short_name}(self ${method.dump_params_name_only_with_comma()});
}
%endfor
%endfor
// END PROXY GENERATED