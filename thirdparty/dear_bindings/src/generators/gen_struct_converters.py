from src import code_dom
from src import utils
from src import conditional_generator
from src.code_dom.common import write_c_line


# Recursively generate code to copy all the members of a struct and any contained by-valuestructs
def generate_field_copies(file, indent, known_by_value_structs, struct, prefix):
    # Emit code to copy each member
    for field in struct.list_directly_contained_children_of_type(code_dom.DOMFieldDeclaration):
        if field.field_type.to_c_string() in known_by_value_structs:
            # This is a by-value struct type, so recurse to copy members of it
            for name in field.names:
                generate_field_copies(file,
                                      indent,
                                      known_by_value_structs,
                                      known_by_value_structs[field.field_type.to_c_string()],
                                      prefix + name + ".")
        else:
            for name in field.names:
                write_c_line(file, indent, "dest." + prefix + name + " = src." + prefix + name + ";")


# Generate code to convert by-value types to/from their CPP version
def generate(dom_root, file, indent=0):

    # Make a list of known by-value structs so we can copy them property
    known_by_value_structs = {}

    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if struct.is_by_value and not struct.is_forward_declaration:
            known_by_value_structs[struct.name] = struct

    write_c_line(file, indent, "// By-value struct conversions")
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        # The original (C++) version of the struct
        original_struct = struct.unmodified_element or struct
        if struct.is_by_value and not struct.is_forward_declaration:
            for to_cpp in [False, True]:
                src_type = ("cimgui::" + struct.name) if to_cpp else \
                    original_struct.get_fully_qualified_name(include_leading_colons=True)
                dest_type = original_struct.get_fully_qualified_name(include_leading_colons=True) if to_cpp else \
                    ("cimgui::" + struct.name)

                function_prefix = "ConvertToCPP_" if to_cpp else "ConvertFromCPP_"

                file.write("\n")
                write_c_line(file, indent, "static inline " +
                             dest_type + " " + function_prefix + struct.name + "(const " + src_type + "& src" + ")")
                write_c_line(file, indent, "{")
                indent += 1

                write_c_line(file, indent, dest_type + " dest;")

                # Emit code to copy each member
                generate_field_copies(file, indent, known_by_value_structs, struct, "")

                write_c_line(file, indent, "return dest;")
                indent -= 1
                write_c_line(file, indent, "}")
