from .common import *
from src import code_dom


# A field declaration
class DOMFieldDeclaration(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.field_type = None
        self.names = []
        self.is_static = False
        self.is_extern = False
        self.is_anonymous = False  # True if the field is anonymous (an implicit field for a nested type declaration)
        self.is_array = []  # One per name, because C
        self.width_specifiers = []  # One per name
        self.array_bounds_tokens = []  # One list of tokens per name
        self.is_imgui_api = False  # Does this use IMGUI_API?
        self.accessibility = None  # The field accessibility
        self.name_alignment = 0  # Column to align name to (for aesthetic purposes)

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()
        dom_element = DOMFieldDeclaration()

        # Parse prefixes
        while True:
            prefix_token = stream.peek_token_of_type(["THING"])
            if prefix_token is None:
                break

            if prefix_token.value == 'IMGUI_API':
                stream.get_token()  # Eat token
                dom_element.is_imgui_api = True
            elif prefix_token.value == 'static':
                stream.get_token()  # Eat token
                dom_element.is_static = True
            elif prefix_token.value == 'extern':
                stream.get_token()  # Eat token
                dom_element.is_extern = True
            else:
                break

        # Parse field type
        dom_element.field_type = code_dom.type.DOMType.parse(context, stream)
        if dom_element.field_type is None:
            stream.rewind(checkpoint)
            return None
        dom_element.field_type.parent = dom_element

        if isinstance(dom_element.field_type, code_dom.functionpointertype.DOMFunctionPointerType):
            # Function pointers contain their own name
            dom_element.names.append(dom_element.field_type.name)
            dom_element.is_array.append(False)
            dom_element.array_bounds_tokens.append(None)
            dom_element.width_specifiers.append(None)

            stream.get_token_of_type(["SEMICOLON"])

            return dom_element
        else:
            while True:
                name_token = stream.get_token_of_type(["THING"])
                if name_token is None:
                    stream.rewind(checkpoint)
                    return None

                dom_element.tokens.append(name_token)
                dom_element.names.append(name_token.value)

                # Check for an array specifier

                if stream.get_token_of_type(["LSQUARE"]) is not None:
                    dom_element.is_array.append(True)
                    token_list = []
                    while True:
                        tok = stream.get_token()
                        if tok is None:
                            stream.rewind(checkpoint)
                            return None
                        if tok.type == 'RSQUARE':
                            break

                        token_list.append(tok)
                    dom_element.array_bounds_tokens.append(token_list)
                else:
                    dom_element.is_array.append(False)
                    dom_element.array_bounds_tokens.append(None)

                # Check for a width specifier

                if stream.get_token_of_type(['COLON']):
                    width_specifier_token = stream.get_token_of_type(['DECIMAL_LITERAL'])
                    if width_specifier_token is None:
                        stream.rewind(checkpoint)
                        return None
                    dom_element.width_specifiers.append(int(width_specifier_token.value))
                else:
                    dom_element.width_specifiers.append(None)

                separator_token = stream.get_token_of_type(["SEMICOLON", "COMMA"])
                if separator_token is None:
                    stream.rewind(checkpoint)
                    return None

                if separator_token.type == 'SEMICOLON':
                    # Field declaration finished
                    return dom_element

    def get_child_lists(self):
        lists = code_dom.element.DOMElement.get_child_lists(self)
        if self.field_type is not None:
            lists.append([self.field_type])
        return lists

    def get_writable_child_lists(self):
        return code_dom.element.DOMElement.get_writable_child_lists(self)

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(self.names[0] if len(self.names) > 0 else leaf_name,
                                                        include_leading_colons)
        else:
            return self.names[0] if len(self.names) > 0 else leaf_name

    # Get the initial (pre-name) part of the declaration. This is a separate function because
    # mod_align_structure_field_names needs it
    def get_prefix_and_type(self, context):
        declaration = self.field_type.to_c_string(context)

        if self.is_imgui_api:
            if context.for_c:
                declaration = "CIMGUI_API " + declaration  # Use CIMGUI_API instead of IMGUI_API as our define here
            else:
                declaration = "IMGUI_API " + declaration

        if self.is_static:
            declaration = "static " + declaration

        # Emit extern if required, but not "extern CIMGUI_API" in C as that expands to "extern extern "C""
        if self.is_extern and (not self.is_imgui_api or not context.for_c):
            declaration = "extern " + declaration

        return declaration

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        declaration = self.get_prefix_and_type(context)

        # Function pointers have the name/etc included
        if not isinstance(self.field_type, code_dom.functionpointertype.DOMFunctionPointerType):
            # Pad declaration to align name if required
            if len(declaration) < self.name_alignment:
                declaration += " " * (self.name_alignment - len(declaration))

            first_name = True
            for i in range(0, len(self.names)):
                if first_name:
                    declaration += " "
                else:
                    declaration += ", "
                declaration += self.names[i]
                if self.is_array[i]:
                    declaration += "["
                    if self.array_bounds_tokens[i] is not None:
                        declaration += collapse_tokens_to_string(self.array_bounds_tokens[i])
                    declaration += "]"
                if self.width_specifiers[i] is not None:
                    declaration += " : " + str(self.width_specifiers[i])
                first_name = False

        write_c_line(file, indent, self.add_attached_comment_to_line(declaration + ";"))

    def __str__(self):
        result = "Field: Type=" + str(self.field_type) + " Names="
        for i in range(0, len(self.names)):
            result += " " + self.names[i]
            if self.is_array[i]:
                result += "["
                if self.array_bounds_tokens[i] is not None:
                    result += collapse_tokens_to_string(self.array_bounds_tokens[i])
                result += "]"
            if self.width_specifiers[i] is not None:
                result += " : " + str(self.width_specifiers[i])
        return result
