from src import code_dom
from src import utils


# This modifier looks for anything preceded with a comment that starts with "[Internal]", and marks all of the
# fields/methods from that point until the next blank line as internal
def apply(dom_root):
    for comment in dom_root.list_all_children_of_type(code_dom.DOMComment):
        if comment.is_attached_comment:
            continue

        if comment.comment_text.startswith("// [Internal]"):
            if comment.is_preceding_comment:
                current = comment.parent  # Preceding comments have the first line they refer to as their parent
            else:
                current = comment
            while (current is not None) and (not isinstance(current, code_dom.DOMBlankLines)):
                current.is_internal = True
                #  For debugging:
                #  utils.append_comment_text(current, " [MARKED-INTERNAL]")
                current = current.parent.get_next_child(current)
