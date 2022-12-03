from src import code_dom
from src import utils


# This modifier adds forward declarations for all structs to the top of the file, to avoid dependency problems
# (that mainly occur as a result of template expansions)
def apply(dom_root):
    forward_declarations = {}

    # Construct forward declarations

    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if (struct.structure_type != "UNION") and \
                (struct.name is not None) and \
                (struct.get_parent_class() is None):  # Only forward declare file-scope, non-anonymous structs
            if struct.name not in forward_declarations:
                if struct.is_forward_declaration:
                    # Struct is forward declared already in the file, so just mark it off in our list as existing
                    # (technically this could cause problems if the existing forward declaration is too late to be
                    # useful, but we'll deal with that if it ever happens)
                    forward_declarations[struct.name] = None
                else:
                    if struct.name not in forward_declarations:
                        # Generate a new forward declaration
                        new_declaration = struct.clone_without_children()
                        new_declaration.is_forward_declaration = True
                        # Remove comments from forward-declarations
                        new_declaration.attached_comment = None
                        new_declaration.pre_comments = []
                        forward_declarations[struct.name] = new_declaration

    # Generate a list of declarations to add

    declarations_to_add = []

    for declaration in forward_declarations.values():
        if declaration is not None:
            declarations_to_add.append(declaration)

    if len(declarations_to_add) == 0:
        return

    # Add an explanatory comment
    comment = code_dom.DOMComment()
    comment.comment_text = "// Auto-generated forward declarations for C header"
    declarations_to_add.insert(0, comment)

    # Add to the file

    insert_point = dom_root.children[0]  # Default to adding at the top of the file if we can't find anywhere else

    # Look for the right section to add these to - if we can, we want to put them in the same place as other
    # forward declarations
    for comment in dom_root.list_all_children_of_type(code_dom.DOMComment):
        if "[SECTION] Forward declarations and basic types" in comment.comment_text:
            insert_point = comment
            # No early-out here because we actually want the /last/ instance of this comment

    if insert_point is not None:
        # Skip down past any whitespace and other comments
        next_line = insert_point.parent.get_next_child(insert_point)
        while isinstance(next_line, code_dom.DOMComment) or isinstance(next_line, code_dom.DOMBlankLines):
            insert_point = next_line
            next_line = insert_point.parent.get_next_child(insert_point)

    insert_point.parent.insert_after_child(insert_point, declarations_to_add)
