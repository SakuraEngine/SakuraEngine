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
    def __init__(self, name, type, offset, comment):
        self.name = name
        self.type = type
        self.getter = None
        self.setter = None
        self.offset = offset
        self.comment = comment


class FunctionDesc(object):
    def __init__(self, retType, fields, isConst, comment):
        self.retType = retType
        self.fields = fields
        self.isConst = isConst
        self.comment = comment

    def getCall(self):
        return ", ".join(["args["+str(i)+"].Convert<"+field.type+">()" for i, field in enumerate(self.fields)])

    def getSignature(self, record):
        return self.retType + "(" + (record.name + "::" if record else "") + "*)(" + str.join(", ",  [x.type for x in self.fields]) + ")" + ("const" if self.isConst else "")


class Function(object):
    def __init__(self, name):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.descs = []


class Record(object):
    def __init__(self, name, guid, fields, methods, bases, comment, fileName):
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
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])

    def allFields(self):
        result = []
        result.extend(self.fields)
        for base in self.bases:
            result.extend(base.allFields())
        return result

    def allMethods(self):
        result = dict(self.methods)
        for base in self.bases:
            for k, v in base.allMethods().items():
                function = result.setdefault(k, Function(k))
                function.descs.extend(v.descs)
        return result


def parseRecord(name, json):
    if not "guid" in json["attrs"]:
        return
    fields = []
    for key, value in json["fields"].items():
        attr = value["attrs"]
        if "no-rtti" in attr:
            continue
        field = Field(key, value["type"],
                      value["offset"], value["comment"])
        fields.append(field)
    functions = parseFunctions(json["methods"])
    bases = []
    for value in json["bases"]:
        bases.append(value)
    return Record(name, json["attrs"]["guid"], fields, functions,
                  bases, json["comment"], json["fileName"])


class Enumerator(object):
    def __init__(self, name, value, comment):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.value = value
        self.comment = comment


class Enum(object):
    def __init__(self, name, guid, underlying_type, enumerators, comment, fileName):
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
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])


def parseEnum(name, json):
    attr = json["attrs"]
    if not "guid" in attr:
        return
    enumerators = []
    for key, value in json["values"].items():
        enumerators.append(Enumerator(
            key, value["value"], value["comment"]))
    return Enum(name, attr["guid"], json["underlying_type"],
                enumerators, json["comment"], json["fileName"])


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

    def resolve_base(self):
        for record in self.records:
            bases = []
            for base in record.bases:
                if base in self.name_to_record:
                    bases.append(self.name_to_record[base])
                else:
                    abort("base %s not reflected" % base)
            record.bases = bases

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
    config = api.lower()+".configure.h"
    api = api.upper()+"_API"
    includes = sys.argv[4:].copy()
    includes.append(root)
    for path in includes:
        metas = glob.glob(os.path.join(path, "**", "*.h.meta"), recursive=True)
        for meta in metas:
            meta = json.load(open(meta))
            for key, value in meta["records"].items():
                db.add_record(parseRecord(key, value))
            for key, value in meta["enums"].items():
                db.add_enum(parseEnum(key, value))
    db.resolve_base()

    data = Binding()
    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    for meta in metas:
        data2 = Binding()
        filename = os.path.split(meta)[1][:-7]
        meta = json.load(open(meta))
        for key, value in meta["records"].items():
            if not "guid" in value["attrs"]:
                continue
            if not "rtti" in value["attrs"]:
                continue
            record = db.name_to_record[key]
            data.headers.add(GetInclude(record.fileName))
            data.records.append(record)
            data2.records.append(record)

        for key, value in meta["enums"].items():
            if not "guid" in value["attrs"]:
                continue
            if not "rtti" in value["attrs"]:
                continue
            enum = db.name_to_enum[key]
            data.headers.add(GetInclude(enum.fileName))
            data.enums.append(enum)
            data2.enums.append(enum)
        if data2.enums or data2.records:
            template = os.path.join(BASE, "rtti.hpp.mako")
            content = render(template, db=data2, api=api,  config=config)
            data.headers.add("%s.rtti.generated.hpp" % filename)
            output = os.path.join(outdir, "%s.rtti.generated.hpp" % filename)
            write(output, content)
    if data.enums or data.records:
        template = os.path.join(BASE, "rtti.cpp.mako")
        content = render(template, db=data)
        output = os.path.join(outdir, "rtti.generated.cpp")
        write(output, content)


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
        os.makedirs(directory)
    open(path, "wb").write(content.encode("utf-8"))

    python_addr = RE_PYTHON_ADDR.search(content)
    if python_addr:
        abort('Found "{}" in {} ({})'.format(
            python_addr.group(0), os.path.basename(path), path))


def abort(message):
    print(message, file=sys.stderr)
    sys.exit(1)


def parseFunctions(dict):
    functionsDict = {}
    for value in dict:
        attr = value["attrs"]
        if "no-rtti" in attr:
            continue
        name = value["name"]
        function = functionsDict.setdefault(name, Function(name))
        fields = []
        for key2, value2 in value["parameters"].items():
            field = Field(key2, value2["type"],
                          value2["offset"], value2["comment"])
            fields.append(field)
        f = FunctionDesc(value["retType"], fields,
                         value["isConst"], value["comment"])
        function.descs.append(f)
    return functionsDict


if __name__ == "__main__":
    main()
