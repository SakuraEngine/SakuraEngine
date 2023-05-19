import os
import re

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_types(self, records):
        return [record for record in records if hasattr(record.attrs, "scriptable")]

    def filter_functions(self, functions):
        return [functions for function in functions if hasattr(function.attrs, "scriptable")]
    
    def escape(self, name):
        if name == "repeat":
            return "repeat_"
        if name == "in":
            return "in_"
        return name
    
    def lua_type(self, name):
        if name == "bool":
            return "boolean"
        elif name == "int":
            return "integer"
        elif name == "float":
            return "number"
        elif name == "const char*":
            return "string"
        elif name == "void *":
            return "any"
        elif name.startswith("skr::resource::TResourceHandle"):
            return "resource_handle"# + name[30:]
        return name.replace("const", "").replace("*", "").replace("&", "").replace("::", "_").strip()
    
    def lua_functor(self, st, function):
        ps = [self.lua_param(name, param) for name, param in vars(function.parameters).items()]
        if(st != ""):
            ps.insert(0, st)
        plist = ", ".join(ps)
        ret = " -> " + (self.lua_type(function.retType) if function.retType != "void" else "nil")
        return "(%s)"%plist + ret
    
    def lua_param(self, name, param):
        if param.isCallback:
            t = self.lua_functor("", param.functor)
        else:
            t = self.lua_type(param.type)
        return self.escape(name) + ": " + t

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "luabind.h.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self, api=args.api+"_API", module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "luabind.cpp.mako")
        itemplate = os.path.join(BASE, "luabind_intelli.lua.mako")
        itemplate2 = os.path.join(BASE, "luabind_intelli.luau.mako")
        icontent = db.render(itemplate, db=db, generator = self, module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        icontent2 = db.render(itemplate2, db=db, generator = self, module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        db.write(os.path.join(args.outdir, "luabind.d.lua"), icontent2)
        db.write(os.path.join(args.outdir, "luabind_intelli.lua"), icontent)
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self, module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        return ""
