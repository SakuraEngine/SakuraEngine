from src import code_dom


# This modifier removes any operator methods
def apply(dom_root):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if function.is_operator:
            function.parent.remove_child(function)
