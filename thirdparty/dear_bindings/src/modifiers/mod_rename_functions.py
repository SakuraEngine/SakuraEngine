from src import code_dom


# This modifier renames functions
# Takes a map mapping old names to new names
def apply(dom_root, name_map):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if function.name in name_map:
            function.name = name_map[function.name]
