import os
import re

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_types(self, records):
        return [record for record in records if hasattr(record.attrs, "trait")]
    
    def filter_functions(self, functions):
        return [function for function in functions if not function.isStatic]

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "trait_object.h.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self, api=args.api+"_API")
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "trait_object.cpp.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums) or self.filter_functions(db.functions): 
            return db.render(template, db=db, generator = self)
        return ""
