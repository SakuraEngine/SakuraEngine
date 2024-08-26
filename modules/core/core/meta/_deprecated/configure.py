import json
import os
import sys
import re
import glob
from mako import exceptions
from mako.template import Template
from pathlib import Path
import itertools


BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))


def main():
    root = sys.argv[1]
    outdir = sys.argv[2]
    api = sys.argv[3]
    module = sys.argv[4]

    template = os.path.join(BASE, "configure.h.mako")
    content = render(template, api=api.upper(), module=module)
    output = os.path.join(outdir, "module.configure.h")
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
