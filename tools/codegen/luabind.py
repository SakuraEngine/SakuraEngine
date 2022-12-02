import os

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
        if self.filter_types(db.records) and self.filter_types(db.enums) and self.filter_functions(db.functions):
            return db.render(template, db=db, generator = self, api=args.api+"_API")
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "luabind.cpp.mako")
        if self.filter_rtti(db.records) and self.filter_types(db.enums) and self.filter_functions(db.functions):
            return db.render(template, db=db, generator = self)
        return ""
