from src import code_dom
from src import utils


# This modifier removes forward declarations of enums (which aren't possible in C)
def apply(dom_root):
    enum_declarations = {}  # Actual enum declarations
    forward_declarations = []  # Forward declarations

    # Make a list of enums and forward declarations
    for enum in dom_root.list_all_children_of_type(code_dom.DOMEnum):
        if enum.is_forward_declaration:
            forward_declarations.append(enum)
        else:
            enum_declarations[enum.name] = enum

    # Chose which approach we're going to use to deal with the forward declarations (move the actual declaration or
    # rewrite the forward declaration as an integer typedef)
    move_enums = False

    if move_enums:
        # This version of the implementation moves the actual enum declaration to where the forward-declaration is.
        # This isn't the most aesthetically pleasing solution because it somewhat mucks with the layout of the header
        # file, but it is arguably the "more correct" one in that the enum remains as an enum.

        for forward_declaration in forward_declarations:
            actual_enum = enum_declarations[forward_declaration.name]

            # Migrate any comments from the forward declaration onto the actual enum
            utils.migrate_comments(forward_declaration, actual_enum)

            # Insert the real declaration
            forward_declaration.parent.insert_after_child(forward_declaration, [actual_enum])

            # Remove the now-redundant forward declaration
            forward_declaration.parent.remove_child(forward_declaration)
    else:
        # This version changes the forward declaration into a typedef to the enum storage type, and then makes
        # the enum declaration itself anonymous (so it only defines the actual enum values). This preserves the file
        # layout and also the defined storage type, but at the cost of the typedef not actually being a real enum.

        for forward_declaration in forward_declarations:
            actual_enum = enum_declarations[forward_declaration.name]

            # Construct a typedef for the enum

            storage_type_str = "int"

            if forward_declaration.storage_type is not None:
                storage_type_str = forward_declaration.storage_type.to_c_string()

            typedef = utils.create_typedef("typedef " + storage_type_str + " " + forward_declaration.name)

            # Insert the typedef and remove the old forward declaration
            utils.migrate_comments(forward_declaration, typedef)
            forward_declaration.parent.insert_after_child(forward_declaration, [typedef])
            forward_declaration.parent.remove_child(forward_declaration)

            # Make the actual enum anonymous
            # (we can't actually remove the name as that will break things elsewhere that want to look up enum details,
            # so instead we use a special flag to suppress the name in the output)
            actual_enum.emit_as_anonymous_for_c = True

            # Add a comment to the enum to make it clear what type is actually is
            utils.append_comment_text(actual_enum, "Forward declared enum type " + actual_enum.name)

