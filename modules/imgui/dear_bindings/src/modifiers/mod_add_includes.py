from src import code_dom
from src import utils


# This modifier adds the listed #includes
# include_filenames should be a list of filenames in include syntax (e.g. "<stdio.h>" or similar)
def apply(dom_root, include_filenames):

    insert_point = dom_root.children[0]  # Default to adding at the top of the file if we can't find anywhere else

    # Look for the right section to add these to - if we can, we want to put them in the same place as other includes
    for comment in dom_root.list_all_children_of_type(code_dom.DOMComment):
        if comment.comment_text == "// Includes":
            insert_point = comment
            # No early-out here because we actually want the /last/ instance of this comment

    if insert_point is not None:
        # Skip down past any whitespace and other comments
        next_line = insert_point.parent.get_next_child(insert_point)
        while isinstance(next_line, code_dom.DOMComment) or \
                isinstance(next_line, code_dom.DOMBlankLines):
            insert_point = next_line
            next_line = insert_point.parent.get_next_child(insert_point)

    for filename in include_filenames:
        include = code_dom.DOMInclude()
        include.tokens = [utils.create_token("#include"), utils.create_token(filename)]

        insert_point.parent.insert_after_child(insert_point, [include])
