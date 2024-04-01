import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_type(self, record):
        if hasattr(record.attrs, "serialize"):
            if "json" in record.attrs.serialize:
                return True
            
    def filter_debug_type(self, record):
        if hasattr(record.attrs, "debug"):
            return True

    def filter_fields(self, fields):
        return [(f, v) for f, v in vars(fields).items() if not hasattr(v.attrs, "transient") and not hasattr(v.attrs, "no-text")]
            
    def filter_types(self, records):
        return [record for record in records if self.filter_type(record) or self.filter_debug_type(record)]

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "json_serialize.h.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums):
            return db.render(template, db=db, generator = self, api=args.api+"_API")
        return ""
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "json_serialize.cpp.mako")
        if self.filter_types(db.records) or self.filter_types(db.enums):
            return db.render(template, db=db, generator = self)
        return ""

