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
    def __init__(self, name, fields, bases):
        self.name = name
        self.luaName = name.replace("::", ".")
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.export_to_c = not "::" in name
        self.fields = fields
        self.bases = bases


class Enumerator(object):
    def __init__(self, name, value):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.value = value
        self.export_to_c = not "::" in name


class Enum(object):
    def __init__(self, name, enumerators):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.enumerators = enumerators
        self.export_to_c = not "::" in name
        for enumerator in enumerators:
            if not enumerator.export_to_c:
                self.export_to_c = False
                break


class Binding(object):
    def __init__(self):
        self.records = []
        self.enums = []
        self.name_to_record = {}
        self.headers = set()

    def add_record(self, record):
        self.records.append(record)
        self.name_to_record[record.name] = record


BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def shouldSkip(value):
    attr = value["attrs"]
    if not "serialize" in attr:
        return True
    serialize = attr["serialize"]
    if isinstance(serialize, list):
        if not "bin" in serialize:
            return True
    else:
        if serialize != "bin":
            return True
    return False


def main():
    db = Binding()
    root = sys.argv[1]
    outdir = sys.argv[2]
    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    for meta in metas:
        meta = json.load(open(meta))
        for key, value in meta["records"].items():
            file = value["fileName"]
            fields = []
            if shouldSkip(value):
                continue
            for key2, value2 in value["fields"].items():
                attr = value2["attrs"]
                if "transient" in attr:
                    continue
                field = Field(key2, value2["type"])
                fields.append(field)
            bases = []
            for value3 in value["bases"]:
                bases.append(value3)
            db.headers.add(GetInclude(file))
            db.add_record(Record(key, fields, bases))
        for key, value in meta["enums"].items():
            attr = value["attrs"]
            file = value["fileName"]
            if shouldSkip(value):
                continue
            db.headers.add(GetInclude(file))
            enumerators = []
            for key2, value2 in value["values"].items():
                enumerators.append(Enumerator(
                    key2, value2["value"]))
            db.enums.append(Enum(key, enumerators))
    if db.records or db.enums:
        template = os.path.join(BASE, "serialize.h.mako")
        content = render(template, db=db)
        output = os.path.join(outdir, "serialize.generated.h")
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
