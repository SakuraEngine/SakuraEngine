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
    def __init__(self, name, guid):
        self.name = name
        self.id = name.replace("::", "_")
        var = str.rsplit(name, "::", 1)
        self.short_name = var[-1]
        self.guid = guid
        self.guidConstant = "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])


class Binding(object):
    def __init__(self):
        self.types = []
        self.headers = set()


BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def main():
    db = Binding()
    root = sys.argv[1]
    outdir = sys.argv[2]
    metas = glob.glob(os.path.join(root, "**", "*.h.meta"), recursive=True)
    print(metas)
    for meta in metas:
        meta = json.load(open(meta))
        for key, value in itertools.chain(meta["records"].items(), meta["enums"].items()):
            file = value["fileName"]
            if not "config" in value["attrs"]:
                continue
            guid = value["attrs"]["guid"]
            db.headers.add(GetInclude(file))
            db.types.append(Type(key, guid))

    template = os.path.join(BASE, "config_resource.cpp.mako")
    content = render(template, db=db)
    output = os.path.join(outdir, "config_resource.generated.cpp")
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
