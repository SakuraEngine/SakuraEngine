from src import code_dom


# This modifier flattens any preprocessor conditionals that match the expression given, either treating it as
# true or false depending on the value of is_true. Currently fairly dumb and will only work if the condition
# matches the string given exactly.
def apply(dom_root, expression, is_true):
    for conditional in dom_root.list_all_children_of_type(code_dom.DOMPreprocessorIf):
        if conditional.get_expression() == expression:
            # Do we want to retain the primary body (i.e. the main chunk) or the secondary (i.e. the "else" chunk)?
            retain_primary = is_true ^ conditional.is_negated
            content_to_retain = conditional.children if retain_primary else conditional.else_children

            if len(content_to_retain) > 0:
                # Promote the retained content to the parent scope
                conditional.parent.insert_after_child(conditional, content_to_retain)

            # ...and remove the conditional itself
            conditional.parent.remove_child(conditional)
