import json
import os
import sys
import re
import glob
from mako import exceptions
from mako.template import Template
from pathlib import Path
import itertools


class Type(object):
    def __init__(self, name, guid, buffer, pin, managed, entityFields):
        self.name = name
        self.id = name.replace("::", "_")
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        if len(var) > 1:
            self.namespace = var[0]
        self.guid = guid
        self.buffer = buffer
        self.pin = pin
        self.managed = managed
        self.entityFields = entityFields
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])


class Binding(object):
    def __init__(self):
        self.types = []
        self.headers = set()


BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def parseRecord(name, json, db):
    file = json["fileName"]
    attr = json["attrs"]
    guid = attr["guid"]
    component = attr["component"]
    buffer = None
    pin = None
    managed = None
    if isinstance(component, dict):
        buffer = component["buffer"]
        pin = "pin" in component
        managed = "managed" in component
    fields = []
    for key, value in json["fields"].items():
        if value["rawType"] != "dual_entity_t":
            continue
        fields.append(str(value["offset"]))
    db.headers.add(GetInclude(file))
    type = Type(name, guid, buffer, pin, managed, fields)
    db.types.append(type)
    return type


def main():
    db = Binding()
    root = sys.argv[1]
    outdir = sys.argv[2]
    api = sys.argv[3]
    config ="module.configure.h"
    api = api.upper()+"_API"
    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    print(metas)

    for meta in metas:
        db2 = Binding()
        filename = os.path.split(meta)[1][:-7]
        meta = json.load(open(meta))
        for key, value in meta["records"].items():
            if not "component" in value["attrs"]:
                continue
            db2.types.append(parseRecord(key, value, db))
        if db2.types:
            template = os.path.join(BASE, "component.hpp.mako")
            content = render(template, db=db2, api=api,  config=config)
            db.headers.add("%s.dual.generated.hpp" % filename)
            output = os.path.join(outdir, "%s.dual.generated.hpp" % filename)
            write(output, content)
    if db.types:
        template = os.path.join(BASE, "component.cpp.mako")
        content = render(template, db=db)
        output = os.path.join(outdir, "component.generated.cpp")
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
