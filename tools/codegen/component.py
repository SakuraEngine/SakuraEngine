import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_record(self, record):
        if hasattr(record.attrs, "component"):
            return True

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "component.hpp.mako")
        return db.render(template, db=db, generator = self, api=args.api+"_API")
    
    def generate_impl(self, db, args):
        template = os.path.join(BASE, "component.cpp.mako")
        return db.render(template, db=db, generator = self)

