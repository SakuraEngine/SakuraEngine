import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_type(self, record):
        if hasattr(record.attrs, "serialize"):
            if "bin" in record.attrs.serialize:
                return True
    
    def filter_types(self, records):
        return [record for record in records if self.filter_type(record)]

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "binary_serialize.h.mako")
        return db.render(template, db=db, generator = self, api=args.api+"_API")
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "binary_serialize.cpp.mako")
        return db.render(template, db=db, generator = self)

