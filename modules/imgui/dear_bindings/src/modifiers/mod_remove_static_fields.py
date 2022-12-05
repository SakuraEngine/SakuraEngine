from src import code_dom


# This modifier removes any static fields
# (on the basis that C doesn't allow them and we'd need to add accessor functions, but right now there aren't any
# static fields that are actually particularly useful to expose)
def apply(dom_root):
    for field in dom_root.list_all_children_of_type(code_dom.DOMFieldDeclaration):
        if field.is_static:
            field.parent.remove_child(field)
