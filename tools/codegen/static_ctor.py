import json
import os
import sys
import re
from mako import exceptions
from mako.template import Template
from pathlib import Path
import glob

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


class Field(object):
    def __init__(self, name, type, offset, comment, static_ctor_exprs):
        self.name = name
        self.type = type
        self.getter = None
        self.setter = None
        self.offset = offset
        self.comment = comment
        self.static_ctor_exprs = static_ctor_exprs

    def realize_expr(self, record, expr):
        expr = re.sub(r"STEXT\(.*?\)", r"\"\0\"", expr)
        expr = expr.replace("$T", self.type)
        expr = expr.replace("$name", "\"%s\""%self.name)
        expr = expr.replace("$field_ptr", "&{}::{}".format(record.name, self.name))
        expr = expr.replace("$Owner", record.name)
        expr = expr.replace("$field", "skr_get_field(skr::type::type_of<{}>::get(), \"{}\")".format(record.name, self.name))
        return expr


class FunctionDesc(object):
    def __init__(self, retType, fields, isConst, comment, static_ctor_exprs):
        self.retType = retType
        self.fields = fields
        self.isConst = isConst
        self.comment = comment
        self.static_ctor_exprs = static_ctor_exprs

    def getCall(self):
        return ", ".join(["args["+str(i)+"].Convert<"+field.type+">()" for i, field in enumerate(self.fields)])

    def getSignature(self, record):
        return self.retType + "(" + (record.name + "::" if record else "") + "*)(" + str.join(", ",  [x.type for x in self.fields]) + ")" + ("const" if self.isConst else "")

    def realize_expr(self, record, expr):
        expr = re.sub(r"STEXT\(.*?\)", r"\"\0\"", expr)
        expr = expr.replace("$name", "\"%s\""%self.name)
        expr = expr.replace("$method_ptr", "&{}::{}".format(record.name, self.name))
        expr = expr.replace("$Owner", record.name)
        expr = expr.replace("$method", "skr_get_method(skr::type::type_of<{}>::get(), \"{}\")".format(record.name, self.name))
        return expr

class Function(object):
    def __init__(self, name):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.descs = []

class Record(object):
    def __init__(self, name, guid, fields, methods, bases, comment, fileName, static_ctor_exprs, post_static_ctor_exprs, shortFileName):
        self.name = name
        self.guid = guid
        self.luaName = name.replace("::", ".")
        self.id = name.replace("::", "_")
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        if len(var) > 1:
            self.namespace = var[0]
        self.fields = fields
        self.methods = methods
        self.bases = bases
        self.comment = comment
        self.fileName = fileName
        self.hashable = False
        self.static_ctor_exprs = static_ctor_exprs
        self.post_static_ctor_exprs = post_static_ctor_exprs
        self.realized_expr = []
        self.shortFileName = shortFileName
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])

    def realize_expr(self, expr : str):
        expr = re.sub(r"STEXT\((.*?)\)", r"\"\0\"", expr)
        expr = expr.replace("$T", self.name)
        if self.bases:
            expr = expr.replace("$Super", self.bases[0])
        expr = expr.replace("$guid", "skr::guid_t{%s}"%self.guidConstant)
        expr = expr.replace("$rtti", "skr::type::type_of<%s>::get()"%self.name)
        return expr


def parseRecord(name, json, shortFileName):
    if not "guid" in json["attrs"]:
        return
    record_static_ctor_exprs = []
    record_post_static_ctor_exprs = []
    for attrkey, attrvalue in json["attrs"].items():
        if attrkey.startswith("StaticCtor"):
            record_static_ctor_exprs.append(attrvalue)
        if attrkey.startswith("PostStaticCtor"):
            record_post_static_ctor_exprs.append(attrvalue)
    fields = []
    for key, value in json["fields"].items():
        attr = value["attrs"]
        field_static_ctor_exprs = []
        for attrkey, attrvalue in attr.items():
            if attrkey.startswith("StaticCtor"):
                field_static_ctor_exprs.append(attrvalue)
        field = Field(key, value["type"],
                      value["offset"], value["comment"], field_static_ctor_exprs)
        fields.append(field)
    functions = parseFunctions(json["methods"])
    bases = []
    for value in json["bases"]:
        bases.append(value)
    return Record(name, json["attrs"]["guid"], fields, functions,
                  bases, json["comment"], json["fileName"], record_static_ctor_exprs, record_post_static_ctor_exprs, shortFileName)

def parseFunctions(dict):
    functionsDict = {}
    for value in dict:
        function_static_ctor_exprs = []
        attr = value["attrs"]
        for attrkey, attrvalue in attr.items():
            if attrkey.startswith("StaticCtor"):
                function_static_ctor_exprs.append(attrvalue)
        name = value["name"]
        function = functionsDict.setdefault(name, Function(name))
        fields = []
        for key2, value2 in value["parameters"].items():
            field = Field(key2, value2["type"],
                          value2["offset"], value2["comment"], [])
            fields.append(field)
        f = FunctionDesc(value["retType"], fields,
                         value["isConst"], value["comment"], function_static_ctor_exprs)
        function.descs.append(f)
    return functionsDict

class Enumerator(object):
    def __init__(self, name, value, comment):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.value = value
        self.comment = comment


class Enum(object):
    def __init__(self, name, guid, underlying_type, enumerators, comment, fileName, static_ctor_exprs, shortFileName):
        self.name = name
        self.guid = guid
        if underlying_type == "unfixed":
            abort(name + " is not fixed enum!")
        self.postfix = ": " + underlying_type
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        if len(var) > 1:
            self.namespace = var[0]
        self.id = name.replace("::", "_")
        self.enumerators = enumerators
        self.comment = comment
        self.fileName = fileName
        self.shortFileName = shortFileName
        self.static_ctor_exprs = static_ctor_exprs
        self.realized_expr = []
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])

            
    def realize_expr(self, expr : str):
        expr = re.sub(r"STEXT\(.*?\)", r"\"\0\"", expr)
        expr = expr.replace("$T", self.name)
        expr = expr.replace("$guid", "skr::guid_t{%s}"%self.guidConstant)
        expr = expr.replace("$rtti", "skr::type::type_of<%s>::get()"%self.name)
        return expr


def parseEnum(name, json, shortFileName):
    attr = json["attrs"]
    enum_static_ctor_exprs = []
    for attrkey, attrvalue in attr.items():
        if attrkey.startswith("StaticCtor"):
            enum_static_ctor_exprs.append(attrvalue)
    if not "guid" in attr:
        return
    enumerators = []
    for key, value in json["values"].items():
        enumerators.append(Enumerator(
            key, value["value"], value["comment"]))
    return Enum(name, attr["guid"], json["underlying_type"],
                enumerators, json["comment"], json["fileName"], enum_static_ctor_exprs, shortFileName)


class Binding(object):
    def __init__(self):
        self.records = []
        self.enums = []
        self.headers = set()


class Database(object):
    def __init__(self):
        self.records = []
        self.enums = []
        self.name_to_record = {}
        self.name_to_enum = {}
        self.headers = set()

    def add_record(self, record):
        if not record:
            return
        self.records.append(record)
        self.name_to_record[record.name] = record

    def add_enum(self, enum):
        if not enum:
            return
        self.enums.append(enum)
        self.name_to_enum[enum.name] = enum


def GetInclude(path):
    return os.path.normpath(path).replace(os.sep, "/")


def main():
    db = Database()
    root = sys.argv[1]
    outdir = sys.argv[2]
    api = sys.argv[3]
    config = "module.configure.h"
    api = api.upper()+"_API"
    includes = sys.argv[4:].copy()
    includes.append(root)
    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    for meta in metas:
        filename = os.path.split(meta)[1][:-7]
        meta = json.load(open(meta))
        for key, value in meta["records"].items():
            db.add_record(parseRecord(key, value, filename))
        for key, value in meta["enums"].items():
            db.add_enum(parseEnum(key, value, filename))
    data = Binding()
    record : Record
    for record in db.records:
        record.realized_expr.extend([record.realize_expr(expr) for expr in record.static_ctor_exprs])
        field : Field
        for field in record.fields:
            record.realized_expr.extend([field.realize_expr(record, expr) for expr in field.static_ctor_exprs])
        method : Function
        for _, method in record.methods.items():
            for desc in method.descs:
                record.realized_expr.extend([desc.realize_expr(record, expr) for expr in desc.static_ctor_exprs])
        record.realized_expr.extend([record.realize_expr(expr) for expr in record.post_static_ctor_exprs])
        if record.realized_expr:
            data.headers.add(GetInclude(record.fileName))
            data.records.append(record)
    enum : Enum
    for enum in db.enums:
        enum.realized_expr.extend([enum.realize_expr(expr) for expr in enum.static_ctor_exprs])
        if enum.realized_expr:
            data.headers.add(GetInclude(enum.fileName))
            data.enums.append(enum)

    if data.records or data.enums:
        template = os.path.join(BASE, "static_ctor.cpp.mako")
        content = render(template, data=data)
        output = os.path.join(outdir, "static_ctor.generated.cpp")
        write(output, content)

def GetInclude(path):
    return os.path.normpath(path).replace(os.sep, "/")

def render(filename, **context):
    try:
        template = Template(
            open(filename, "rb").read(),
            filename=filename,
            input_encoding="utf8",
            strict_undefined=True,
        )
        return template.render(**context)
    except Exception:
        # Uncomment to see a traceback in generated Python code:
        # raise
        abort(exceptions.text_error_template().render())


def write(path, content):
    RE_PYTHON_ADDR = re.compile(r"<.+? object at 0x[0-9a-fA-F]+>")
    directory = os.path.dirname(path)
    if not os.path.exists(directory):
        os.makedirs(directory, exist_ok=True)
    open(path, "wb").write(content.encode("utf-8"))

    python_addr = RE_PYTHON_ADDR.search(content)
    if python_addr:
        abort('Found "{}" in {} ({})'.format(
            python_addr.group(0), os.path.basename(path), path))


def abort(message):
    print(message, file=sys.stderr)
    sys.exit(1)


if __name__ == "__main__":
    main()