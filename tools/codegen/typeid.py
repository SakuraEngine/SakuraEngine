import os

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        pass

    def filter_type(self, record):
        return hasattr(record.attrs, "guid")
    
    def filter_types(self, records):
        return [record for record in records if self.filter_type(record)]

    def generate_forward(self, db, args):
        template = os.path.join(BASE, "typeid.hpp.mako")
        return db.render(template, db=db, generator = self)

