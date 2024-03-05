import os
from typing import List

def record_enable_rttr(record):
    if (hasattr(record.attrs, "rttr")):
        if (not hasattr(record.attrs, "guid")):
            raise Exception(f"if record enable rttr, it must have guid, record: {record.name}")
        if (isinstance(record.attrs.rttr, bool)):
            return record.attrs.rttr # 简写
        elif (hasattr(record.attrs.rttr, "enable")): 
            return record.attrs.rttr.enable 
    return hasattr(record.attrs, "guid") # default enable if has [guid] field

def record_enable_reflect_bases(record):
    if (hasattr(record.attrs, "rttr")):
        if (not hasattr(record.attrs.rttr, "reflect_bases")): 
            return True # default enable
        else:  
            return record.attrs.rttr.reflect_bases
    return record_enable_rttr(record)

def record_exclude_bases(record):
    if (hasattr(record.attrs, "rttr")):
        if (not hasattr(record.attrs.rttr, "exclude_bases")): 
            return [] # default empty
        else:
            return record.attrs.rttr.exclude_bases 
    return [] 

def record_enable_reflect_fields(record):
    if (hasattr(record.attrs, "rttr")):
        if (not hasattr(record.attrs.rttr, "reflect_fields")): 
            return True # default disable
        else:  
            return record.attrs.rttr.reflect_fields
    return False

def record_enable_reflect_methods(record):
    if (hasattr(record.attrs, "rttr")):
        if (not hasattr(record.attrs.rttr, "reflect_methods")): 
            return True # default disable
        else:  
            return record.attrs.rttr.reflect_methods
    return False

def field_enable_rttr(field, default_enable):
    if (hasattr(field.attrs, "rttr")):
        if (isinstance(field.attrs.rttr, bool)):
            return field.attrs.rttr # 简写
        elif (not hasattr(field.attrs.rttr, "enable")): 
            return default_enable # use record default value
        else:
            return field.attrs.rttr.enable

def method_enable_rttr(method, default_enable):
    if (hasattr(method.attrs, "rttr")):
        if (isinstance(method.attrs.rttr, bool)):
            return method.attrs.rttr # 简写
        elif (not hasattr(method.attrs.rttr, "enable")): 
            return default_enable # use record default value
        else:
            return method.attrs.rttr.enable

def enum_enable_rttr(enum):
    return hasattr(enum.attrs, "guid")

class CodegenRecord(object):
    def __init__(self):
        self.bases : List = []
        self.fields : List = []
        self.methods : List = []
        self.attrs : object = None
        self.name : str = None
        self.id : str = None

    def load(self, record):
        exclude_bases = record_exclude_bases(record)
        default_reflect_fields = record_enable_reflect_fields(record)
        default_reflect_methods = record_enable_reflect_methods(record)

        # bases
        if (record_enable_reflect_bases(record)):
            self.bases = [base for base in record.bases if not base in exclude_bases]

        # fields & methods
        self.fields = [(name, field) for (name, field) in vars(record.fields).items() if field_enable_rttr(field, default_reflect_fields)]
        self.methods = [method for method in record.methods if method_enable_rttr(method, default_reflect_methods)]

        # attrs
        self.attrs = record.attrs
        self.name = record.name
        self.id = record.id

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
class Generator(object):
    def __init__(self):
        self.records: List[CodegenRecord] = []
        self.enums: List = []

    def load(self, db):
        self.records.clear()
        self.enums.clear()
        
        # load records
        for record in db.records:
            if (record_enable_rttr(record)):
                codegen_record = CodegenRecord()
                codegen_record.load(record)
                self.records.append(codegen_record)

        # load enums
        for enum in db.enums:
            if (enum_enable_rttr(enum)):
                self.enums.append(enum)


    def generate_forward(self, db, args):
        self.load(db)
        template = os.path.join(BASE, "rttr.hpp.mako")
        return db.render(template, db=db, generator = self, api=args.api+"_API", module=args.module)
    
    def generate_impl(self, db, args):
        self.load(db)
        template = os.path.join(BASE, "rttr.cpp.mako")
        return db.render(template, db=db, generator = self, module=args.module)
