from src import code_dom
from src import utils
import json


# Add comments attached to an element to the dictionary given
def add_comments(element, root):
    comments_root = {}
    had_any_comments = False
    if element.pre_comments:
        preceding_root = []
        comments_root["preceding"] = preceding_root
        for comment in element.pre_comments:
            preceding_root.append(comment.to_c_string())
            had_any_comments = True
    if element.attached_comment:
        comments_root["attached"] = element.attached_comment.to_c_string()
        had_any_comments = True

    if had_any_comments:
        root["comments"] = comments_root


# Add preprocessor conditional information for an element to the dictionary given
def add_preprocessor_conditionals(element, root):
    conditionals_root = []
    had_any_conditionals = False

    conditionals = utils.get_preprocessor_conditionals(element)

    for conditional in conditionals:
        if conditional.is_include_guard:
            continue  # Don't include include guards

        expression = code_dom.common.collapse_tokens_to_string(conditional.expression_tokens)

        if conditional.is_ifdef and conditional.is_negated and (expression == "IMGUI_DISABLE"):
            # Semi-hack - don't clutter up the metadata with "#ifndef IMGUI_DISABLE" as it's kinda redundant
            continue

        conditional_root = {}
        conditionals_root.append(conditional_root)

        is_in_else_block = conditional.is_element_in_else_block(element)

        if conditional.is_ifdef:
            if conditional.is_negated ^ is_in_else_block:
                conditional_root["condition"] = "ifndef"
            else:
                conditional_root["condition"] = "ifdef"
        else:
            if is_in_else_block:
                conditional_root["condition"] = "ifnot"
            else:
                conditional_root["condition"] = "if"
        conditional_root["expression"] = expression
        had_any_conditionals = True

    if had_any_conditionals:
        root["conditionals"] = conditionals_root


# Add internal flag if present
def add_internal_flag(element, root):

    # Note that this is more of a hint than a prohibition, and it may well be desirable for users to have access to
    # elements marked with this flag (but ideally marked in some way so they are aware that the functionality isn't
    # part of the primary API)
    if element.is_internal:
        root["is_internal"] = True


# Emit data for a single function pointer type
def emit_function_pointer_type(function_ptr):
    result = {}

    result["flavour"] = "function_pointer"

    if function_ptr.return_type is not None:
        result["return_type"] = emit_type(function_ptr.return_type)

    arguments_root = []
    result["arguments"] = arguments_root

    for argument in function_ptr.arguments:
        if argument.is_implicit_default:
            continue  # Don't emit implicit default arguments
        arguments_root.append(emit_function_argument(argument))

    return result


# Emit data for a single type
def emit_type(type_info):
    result = {}

    result["declaration"] = type_info.to_c_string()

    if isinstance(type_info, code_dom.DOMFunctionPointerType):
        # Special case for function pointers - we want to include a parsed version as well
        result["type_details"] = emit_function_pointer_type(type_info)

    return result


# Emit data for an enum element
def emit_enum_element(enum):
    result = {}

    result["name"] = enum.name
    if enum.value_tokens is not None:
        result["value"] = code_dom.common.collapse_tokens_to_string(enum.value_tokens)

    add_comments(enum, result)
    add_preprocessor_conditionals(enum, result)
    add_internal_flag(enum, result)

    return result


# Emit data for an enum
def emit_enum(enum):
    result = {}

    result["name"] = enum.name
    result["original_fully_qualified_name"] = enum.get_original_fully_qualified_name()
    if enum.storage_type is not None:
        result["storage_type"] = emit_type(enum.storage_type)

    elements_root = []
    result["elements"] = elements_root

    for element in enum.list_all_children_of_type(code_dom.DOMEnumElement):
        elements_root.append(emit_enum_element(element))

    add_comments(enum, result)
    add_preprocessor_conditionals(enum, result)
    add_internal_flag(enum, result)

    return result

# Emit data for a single typedef
def emit_typedef(typedef):
    result = {}

    result["name"] = typedef.name
    result["type"] = emit_type(typedef.type)

    add_comments(typedef, result)
    add_preprocessor_conditionals(typedef, result)
    add_internal_flag(typedef, result)

    return result


# Emit data for a single field
def emit_field(field):
    result = {}

    names_root = []
    result["names"] = names_root

    # Emit all the field names
    for i in range(0, len(field.names)):
        name_root = {}
        names_root.append(name_root)

        name_root["name"] = field.names[i]
        name_root["is_array"] = field.is_array[i]
        if field.is_array[i]:
            name_root["array_bounds"] = code_dom.common.collapse_tokens_to_string(field.array_bounds_tokens[i])

        if field.width_specifiers[i] is not None:
            name_root["width"] = str(field.width_specifiers[i])

    result["is_anonymous"] = field.is_anonymous

    # Emit the type
    result["type"] = emit_type(field.field_type)

    add_comments(field, result)
    add_preprocessor_conditionals(field, result)
    add_internal_flag(field, result)

    return result


# Walk into a container (initially a struct) and emit field declarations for any fields found
# Avoid recursing into nested structs (as those don't contribute fields to their container)
def emit_struct_field_list(container, fields_root):
    # It is important that we preserve ordering here (so we can't, for example, emit all fields first and then nested
    # structs, as those structs could be implicit field declarations)
    for child in container.children:
        if isinstance(child, code_dom.DOMFieldDeclaration):
            # Regular fields
            fields_root.append(emit_field(child))
        elif isinstance(child, code_dom.DOMClassStructUnion):
            # Nested structs

            # If the struct is anonymous, then it needs a dummy field emitted for it
            # This is technically slightly wrong, as you could have a named struct that is also an implicit field
            # declaration, but the parser doesn't currently support that case (and it isn't exactly common practice
            # in C++ AFAIK), so for now we assume that only anonymous structs fit this pattern.
            if child.is_anonymous:
                dummy_field = code_dom.DOMFieldDeclaration()
                dummy_field.names = [child.name]
                dummy_field.is_array = [False]
                dummy_field.width_specifiers = [None]
                dummy_field.is_anonymous = child.is_anonymous  # Technically wrong, but see above
                # We intentionally don't use utils.create_type() here because if this is an anonymous struct our
                # synthetic name will (deliberately) contain characters that aren't valid in a C++ typename
                dummy_type = code_dom.DOMType()
                dummy_type.tokens = [utils.create_token(child.name)]
                dummy_field.field_type = dummy_type
                fields_root.append(emit_field(dummy_field))
        else:
            # If we find anything else, recurse into it to look for fields (as it may be a preprocessor declaration
            # or similar)
            emit_struct_field_list(child, fields_root)


# Emit data for a single struct
def emit_struct(struct):
    result = {}

    result["name"] = struct.name
    result["original_fully_qualified_name"] = struct.get_original_fully_qualified_name()
    result["type"] = struct.structure_type.lower()  # Lowercase this for consistency with C
    result["by_value"] = struct.is_by_value
    result["forward_declaration"] = struct.is_forward_declaration
    result["is_anonymous"] = struct.is_anonymous

    fields_root = []
    result["fields"] = fields_root

    emit_struct_field_list(struct, fields_root)

    add_comments(struct, result)
    add_preprocessor_conditionals(struct, result)
    add_internal_flag(struct, result)

    return result


# Emit data for a single function argument
def emit_function_argument(argument):
    result = {}

    if argument.name is not None:
        result["name"] = argument.name
    if argument.arg_type is not None:
        result["type"] = emit_type(argument.arg_type)
    result["is_array"] = argument.is_array
    result["is_varargs"] = argument.is_varargs
    if argument.is_array:
        result["array_bounds"] = str(argument.array_bounds)
    if argument.default_value_tokens is not None:
        result["default_value"] = code_dom.common.collapse_tokens_to_string(argument.default_value_tokens)

    return result


# Emit data for a single function
def emit_function(function):
    result = {}

    result["name"] = function.name
    result["original_fully_qualified_name"] = function.get_original_fully_qualified_name()

    if function.return_type is not None:
        result["return_type"] = emit_type(function.return_type)

    arguments_root = []
    result["arguments"] = arguments_root

    for argument in function.arguments:
        if argument.is_implicit_default:
            continue  # Don't emit implicit default arguments
        arguments_root.append(emit_function_argument(argument))

    result["is_default_argument_helper"] = \
        function.is_default_argument_helper  # True for functions that are variants of existing functions but with
    #                                          some of the arguments removed (to emulate C++ default argument
    #                                          behaviour). If you are writing bindings for a language that supports
    #                                          default arguments then you probably want to ignore these.
    result["is_manual_helper"] = function.is_manual_helper  # True for functions that aren't in the C++ API originally
    #                                                         but have been added manually here to provide helpful
    #                                                         extra functionality
    result["is_imstr_helper"] = function.is_imstr_helper  # True for functions that have been added to provide variants
    #                                                       of ImStr-taking functions that accept const char* arguments
    #                                                       instead (and thus should probably be ignored by bindings
    #                                                       that handle ImStr)
    result["has_imstr_helper"] = function.has_imstr_helper  # True for functions that accept ImStr as an argument but
    #                                                         have had a helper generated that accepts const char*
    #                                                         instead. If you are writing bindings that use const char*
    #                                                         instead of ImStr then you probably want to ignore these.

    add_comments(function, result)
    add_preprocessor_conditionals(function, result)
    add_internal_flag(function, result)

    return result


def emit_define(define):
    result = {}

    result["name"] = define.name
    if define.content is not None:
        content = define.content
        # Remove ()s around value if present
        if content.startswith('(') and content.endswith(')'):
            content = content[1:len(content)-1]
        result["content"] = content

    add_comments(define, result)
    add_preprocessor_conditionals(define, result)
    add_internal_flag(define, result)

    return result


# Write metadata about our file to a JSON file
def generate(dom_root, file):
    metadata_root = {}

    # Emit defines
    defines_root = []
    metadata_root["defines"] = defines_root

    for define in dom_root.list_all_children_of_type(code_dom.DOMDefine):
        if not define.exclude_from_metadata:

            # Don't include defines with no actual content
            if define.content is None:
                continue

            # Don't include function-style defines
            if "(" in define.name:
                continue

            defines_root.append(emit_define(define))

    # Emit enums
    enums_root = []
    metadata_root["enums"] = enums_root

    for enum in dom_root.list_all_children_of_type(code_dom.DOMEnum):
        if not enum.exclude_from_metadata and not enum.is_forward_declaration:
            enums_root.append(emit_enum(enum))

    # Emit typedefs
    typedefs_root = []
    metadata_root["typedefs"] = typedefs_root

    for typedef in dom_root.list_all_children_of_type(code_dom.DOMTypedef):
        if not typedef.exclude_from_metadata:
            typedefs_root.append(emit_typedef(typedef))

    # Emit struct declarations
    structs_root = []
    metadata_root["structs"] = structs_root

    # Make a list of all structs we have full definitions for
    structs_with_definitions = {}
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if not struct.is_forward_declaration:
            structs_with_definitions[struct.name] = True

    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        # We want to emit forward declarations IFF they don't have a corresponding actual declaration
        # (because the consumer of the JSON file may want to know about the existence of undefined structs,
        # but there's no point in emitting data for a forward declaration that also has a real definition
        # elsewhere in the file)
        if struct.is_forward_declaration and (struct.name in structs_with_definitions):
            continue

        if not struct.exclude_from_metadata:
            structs_root.append(emit_struct(struct))

    # Emit function declarations
    functions_root = []
    metadata_root["functions"] = functions_root

    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if not function.exclude_from_metadata:
            functions_root.append(emit_function(function))

    # Write JSON to file
    json.dump(metadata_root, file, indent=4)
