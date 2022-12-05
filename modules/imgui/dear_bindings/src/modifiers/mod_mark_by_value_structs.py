from src import code_dom
from src import utils


# This modifier adds a marker to structs that should be treated as pass-by-value, which subsequent modifiers
# (and the code generator) can use
def apply(dom_root, by_value_structs):
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if struct.name in by_value_structs:
            struct.is_by_value = True
