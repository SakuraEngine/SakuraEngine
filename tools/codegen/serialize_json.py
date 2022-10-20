import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_type(self, record):
        if hasattr(record.attrs, "serialize"):
            if "json" in record.attrs.serialize:
                return True

    def filter_fields(self, fields):
        return [(f, v) for f, v in vars(fields).items() if not hasattr(v.attrs, "transient")]
            

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "json_serialize.h.mako")
        return db.render(template, db=db, generator = self, api=args.api+"_API")
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "json_serialize.cpp.mako")
        return db.render(template, db=db, generator = self)

