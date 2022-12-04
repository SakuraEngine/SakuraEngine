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

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "luabind.h.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self, api=args.api+"_API", module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "luabind.cpp.mako")
        itemplate = os.path.join(BASE, "luabind_intelli.lua.mako")
        icontent = db.render(itemplate, db=db, generator = self, module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        db.write(os.path.join(args.outdir, "luabind_intelli.lua"), icontent)
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self, module = re.sub(r'(?<!^)(?=[A-Z])', '_', args.module).lower())
        return ""
