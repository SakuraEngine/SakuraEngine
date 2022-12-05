from src import code_dom


# This modifier removes any typedefs that are left inside classes/structs
# (since C doesn't allow that, but fortunately we know none are relevant)
def apply(dom_root):
    for typedef in dom_root.list_all_children_of_type(code_dom.DOMTypedef):
        if typedef.get_parent_class() is not None:
            typedef.parent.remove_child(typedef)
