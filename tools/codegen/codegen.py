import glob
import importlib
import importlib.util
import argparse
import json
import os
import re
import sys
from types import SimpleNamespace
from mako import exceptions
from mako.template import Template
BASE = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))

def abort(message):
    print(message, file=sys.stderr)
    sys.exit(1)

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

class Enumerator(object):
    def __init__(self, name, value):
        self.name = name
        self.short_name = str.rsplit(name, "::", 1)[-1]
        self.value = value
        self.export_to_c = not "::" in name

class Enum(object):
    def __init__(self, name, underlying_type, enumerators, fileName):
        
        self.enumerators = enumerators
        self.fileName = fileName
        for enumerator in enumerators:
            if not enumerator.export_to_c:
                self.export_to_c = False
                break

class MetaDatabase(object):
    def __init__(self):
        self.records = []
        self.enums = []
        self.name_to_record = {}
        self.name_to_enum = {}
        self.includes = set()

    def add_include(self, path):
        self.includes.add(os.path.normpath(path).replace(os.sep, "/"))

    def set_names(self, type, name):
        type.name = name
        type.id = name.replace("::", "_")
        type.luaName = name.replace("::", ".")
        var = str.rsplit(name, "::", 1)
        type.short_name = var[-1]
        if len(var) > 1:
            type.namespace = var[0]

    def load_meta_file(self, file, add):
        try:
            meta = json.load(open(file), object_hook=lambda d: SimpleNamespace(**d))
        except json.decoder.JSONDecodeError as e:
            print(e)
            abort(file)
        for name, record in vars(meta.records).items():
            self.set_names(record, name)
            if add:
                self.records.append(record)
                self.add_include(record.fileName)
            self.name_to_record[record.name] = record
        for name, enum in vars(meta.enums).items():
            self.set_names(enum, name)
            if add:
                self.enums.append(enum)
                self.add_include(enum.fileName)
            self.name_to_enum[enum.name] = enum

    def render(self, filename, **context):
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

    def generate_forward(self, args):
        template = os.path.join(BASE, "forward.h.mako")
        return self.render(template, db=self, config = args.config)
    def generate_impl(self):
        template = os.path.join(BASE, "impl.cpp.mako")
        return self.render(template, db=self)
    def guid_constant(self, type):
        guid = type.attrs.guid
        return "0x{}, 0x{}, 0x{}, {{0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}, 0x{}}}".format(
            guid[0:8], guid[9:13], guid[14:18], guid[19:21], guid[21:23], guid[24:26], guid[26:28], guid[28:30], guid[30:32], guid[32:34], guid[34:36])
    def short_name(self, name):
        return str.rsplit(name, "::", 1)[-1]
    def call_expr(self, desc):
        return ", ".join(["args["+str(i)+"].Convert<"+field.type+">()" for i, (name, field) in enumerate(vars(desc.parameters).items())])
    def signature(self, record, desc):
        return self.retType + "(" + (record.name + "::" if record else "") + "*)(" + str.join(", ",  [x.type for name, x in vars(desc.fields).items()]) + ")" + ("const" if desc.isConst else "")



def load_generator(i, path):
    spec = importlib.util.spec_from_file_location("Generator%d"%i, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return getattr(module, "Generator")()

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

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('generators', help="generator file list.", nargs="*")
    parser.add_argument('-root', help="root directory of meta files.", required=True, type=str)
    parser.add_argument('-outdir', help="output directory.", required=True, type=str)
    parser.add_argument("-api", help="api name.", required=True, type=str)
    parser.add_argument("-module", help="module name.", required=True, type=str)
    parser.add_argument("-config", help="config file name.", required=True)
    parser.add_argument("-includes", help="include directory list.", nargs="+")
    args = parser.parse_args()

    generators = []
    for i, x in enumerate(args.generators):
        generators.append(load_generator(i, x))

    db = MetaDatabase()
    header_dbs = []
    
    metas = glob.glob(os.path.join(args.root, "**", "*.h.meta"), recursive=True)
    for meta in metas:
        db.load_meta_file(meta, True)
        header_db = MetaDatabase()
        header_db.load_meta_file(meta, True)
        header_db.relative_path = os.path.relpath(meta, args.root)
        header_db.file_id = "FID_" + re.sub(r'\W+', '_', header_db.relative_path)
        header_dbs.append(header_db)
    
    if args.includes:
        for path in args.includes:
            metas = glob.glob(os.path.join(path, "**", "*.h.meta"), recursive=True)
            for meta in metas:
                db.load_meta_file(meta, False)
    
    # gen headers
    for header_db in header_dbs:
        forward_content : str = header_db.generate_forward(args)
        for generator in generators:
            if hasattr(generator, "generate_forward"):
                forward_content = forward_content + generator.generate_forward(header_db, args)
        generated_header = re.sub(r"(.*?)\.(.*?)\.meta", r"\g<1>.generated.\g<2>", header_db.relative_path)
        write(os.path.join(args.outdir, generated_header), forward_content)

    impl_content : str = db.generate_impl()
    for generator in generators:
        if hasattr(generator, "generate_impl"):
            impl_content = impl_content + generator.generate_impl(db, args)

    write(os.path.join(args.outdir, "generated.cpp"), impl_content)


    
    