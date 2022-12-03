from src import code_dom


# This modifier removes any empty (nothing except whitespace and/or comments) preprocessor conditionals
def apply(dom_root):
    for conditional in dom_root.list_all_children_of_type(code_dom.DOMPreprocessorIf):
        has_any_meaningful_children = False

        for child in conditional.list_directly_contained_children():
            if not isinstance(child, code_dom.DOMComment) and not isinstance(child, code_dom.DOMBlankLines):
                has_any_meaningful_children = True
                break

        if not has_any_meaningful_children:
            conditional.parent.remove_child(conditional)
