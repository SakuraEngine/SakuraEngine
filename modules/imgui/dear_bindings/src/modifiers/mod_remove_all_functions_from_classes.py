from src import code_dom


# This modifier removes all the functions from the classes specified
def apply(dom_root, class_names):
    for class_element in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if class_element.get_fully_qualified_name() not in class_names:
            continue

        for function in class_element.list_directly_contained_children_of_type(code_dom.DOMFunctionDeclaration):
            function.parent.remove_child(function)
