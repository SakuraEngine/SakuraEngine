import json
import os
import sys
import re
import glob
from mako import exceptions
from mako.template import Template
from pathlib import Path


class Field(object):
    def __init__(self, name, type):
        self.name = name
        self.type = type
        self.getter = None
        self.setter = None


class Record(object):
    def __init__(self, name, fields, bases, fileName):
        self.name = name
        self.luaName = name.replace("::", ".")
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        if len(var) > 1:
            self.namespace = var[0]
        self.export_to_c = not "::" in name
        self.fields = fields
        self.bases = bases
        self.fileName = fileName

    def allFields(self):
        result = []
        result.extend(self.fields)
        for base in self.bases:
            result.extend(base.allFields())
        return result


def parseRecord(name, json):
    fields = []
    if shouldSkip(json):
        return
    for key, value in json["fields"].items():
        attr = value["attrs"]
        if "transient" in attr:
            continue
        field = Field(key, value["type"])
        fields.append(field)
    bases = []
    for value in json["bases"]:
        bases.append(value)
    return Record(name, fields, bases, json["fileName"])


class Enumerator(object):
    def __init__(self, name, value):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.value = value
        self.export_to_c = not "::" in name


class Enum(object):
    def __init__(self, name, underlying_type, enumerators, fileName):
        self.name = name
        if underlying_type == "unfixed":
            abort(name + " is not fixed enum!")
        self.postfix = ": " + underlying_type
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        if len(var) > 1:
            self.namespace = var[0]
        self.enumerators = enumerators
        self.export_to_c = not "::" in name
        self.fileName = fileName
        for enumerator in enumerators:
            if not enumerator.export_to_c:
                self.export_to_c = False
                break


def parseEnum(name, json):
    if shouldSkip(json):
        return
    enumerators = []
    for key2, value2 in json["values"].items():
        enumerators.append(Enumerator(
            key2, value2["value"]))
    return Enum(name, json["underlying_type"], enumerators, json["fileName"])


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
                    abort("serialize_json: baseclass %s not reflected" % base)
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


class Binding(object):
    def __init__(self):
        self.records = []
        self.enums = []
        self.headers = set()


BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def shouldSkip(value):
    attr = value["attrs"]
    if not "serialize" in attr:
        return True
    serialize = attr["serialize"]
    if isinstance(serialize, list):
        if not "json" in serialize:
            return True
    else:
        if serialize != "json":
            return True
    return False


def main():
    db = Database()
    data = Binding()
    root = sys.argv[1]
    outdir = sys.argv[2]
    api = sys.argv[3]
    config = "module.configure.h"
    api = api.upper()+"_API"
    includes = sys.argv[4:].copy()
    includes.append(root)

    for path in includes:
        metas = glob.glob(os.path.join(path, "**", "*.h.meta"), recursive=True)
        for meta in metas:
            try:
                meta = json.load(open(meta))
            except json.decoder.JSONDecodeError as e:
                print(e)
                abort(meta)
            for key, value in meta["records"].items():
                db.add_record(parseRecord(key, value))
            for key, value in meta["enums"].items():
                db.add_enum(parseEnum(key, value))

    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    for meta in metas:
        meta = json.load(open(meta))
        for key, value in meta["records"].items():
            if key in db.name_to_record:
                record = db.name_to_record[key]
                data.records.append(record)
                data.headers.add(GetInclude(record.fileName))
        for key, value in meta["enums"].items():
            if key in db.name_to_enum:
                enum = db.name_to_enum[key]
                data.enums.append(enum)
                data.headers.add(GetInclude(enum.fileName))
                
    db.resolve_base()
    if data.enums or data.records:
        template = os.path.join(BASE, "json_serialize.cpp.mako")
        content = render(template, db=data)
        output = os.path.join(outdir, "json_serialize.generated.cpp")
        write(output, content)
        template = os.path.join(BASE, "json_serialize.h.mako")
        content = render(template, db=data, api=api, config=config)
        output = os.path.join(outdir, "json_serialize.generated.h")
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
