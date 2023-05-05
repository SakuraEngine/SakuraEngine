import json
import os
import re
import sys
from types import SimpleNamespace
from mako import exceptions
from mako.template import Template

def abort(message):
    print(message, file=sys.stderr)
    sys.exit(1)


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

BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))

if __name__ == '__main__':
    meta = json.load(open(os.path.join(BASE, "../../thirdparty/dear_bindings/cimgui.json"), encoding='utf-8'), object_hook=lambda d: SimpleNamespace(**d))
    binding = render(os.path.join(BASE, "luabind_imgui.cpp.mako"), meta=meta)
    write(os.path.join(BASE, "../../modules/imgui/src/build.luabind.cpp"), binding)
    intelligence = render(os.path.join(BASE, "luabind_imgui_intelli.lua.mako"), meta=meta)
    write(os.path.join(BASE, "../../modules/imgui/script/luabind_intelli.lua"), intelligence)
