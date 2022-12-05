from src import code_dom


# This modifier removes constructions and destructors that would result in heap allocations
# (i.e. those not on value types)
def apply(dom_root):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if function.is_constructor or function.is_destructor:
            parent_class = function.get_parent_class()
            if (parent_class is not None) and (not parent_class.is_by_value):
                function.parent.remove_child(function)
