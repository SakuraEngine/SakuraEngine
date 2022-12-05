from src import code_dom
from src import utils


# This modifier excludes the named defines from being emitted in the metadata
def apply(dom_root, names):
    for define in dom_root.list_all_children_of_type(code_dom.DOMDefine):
        if define.name in names:
            define.exclude_from_metadata = True
