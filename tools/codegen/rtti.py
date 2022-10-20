import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass


    def filter_rtti(self, records):
        return [record for record in records if hasattr(record.attrs, "rtti")]

    def filter_no_rtti(self, records):
        return [record for record in records if not hasattr(record.attrs, "no-rtti")]

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "rtti.hpp.mako")
        return db.render(template, db=db, generator = self, api=args.api+"_API")
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "rtti.cpp.mako")
        return db.render(template, db=db, generator = self)
