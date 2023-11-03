
// BEGIN TRAIT GENERATED


%for record in generator.filter_types(db.records):
%for method in record.methods:
    <%
        params_expr = ", ".join("{} {}".format(parameter.type, name) for name, parameter in vars(method.parameters).items())
        args_expr = ", ".join(name for name, parameter in vars(method.parameters).items())
        if(args_expr != ""):
            args_expr = ", " + args_expr
        isConst = "const" if method.isConst else ""
    %>
    ${method.retType} ${method.name}(${params_expr}) ${isConst}
    {
        return vtable->${db.short_name(method.name)}(self ${args_expr});
    }
%endfor
%endfor