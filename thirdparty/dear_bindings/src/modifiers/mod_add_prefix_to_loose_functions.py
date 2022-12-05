from src import code_dom
from src import utils


# This modifier adds a name prefix to any "loose" functions (i.e. not a member of a class/struct/namespace/etc)
def apply(dom_root, prefix):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if "::" not in function.get_fully_qualified_name(return_fqn_even_for_member_functions=True):
            function.name = prefix + function.name
