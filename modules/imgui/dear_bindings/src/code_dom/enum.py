from .common import *
from src import code_dom


# An enum
class DOMEnum(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None
        self.is_enum_class = False
        self.is_forward_declaration = False
        self.emit_as_anonymous_for_c = False  # If this is true, then the enum will be emitted as anonymous in C
        #                                       (used for forward declared enums that have been converted to typedefs)
        self.storage_type = None  # The storage type (int/byte/etc) of the enum, if specified

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()
        dom_element = DOMEnum()
        tok = stream.get_token_of_type(['ENUM'])

        if stream.get_token_of_type(['CLASS']):
            dom_element.is_enum_class = True

        name_tok = stream.get_token_of_type(['THING'])

        dom_element.tokens = [tok, name_tok]

        if stream.get_token_of_type(['COLON']) is not None:
            # If followed by a : then there is an explicit storage type declaration
            dom_element.storage_type = code_dom.DOMType.parse(context, stream)

        dom_element.name = name_tok.value

        # We need a custom content parser for enums
        old_content_parser = context.current_content_parser
        context.current_content_parser = lambda: DOMEnum.parse_content(context, stream)

        if stream.get_token_of_type(['LBRACE']) is not None:
            while True:
                tok = stream.peek_token()
                if tok.type == 'RBRACE':
                    stream.get_token()  # Eat the closing brace
                    break

                element = context.current_content_parser()

                if element is not None:
                    if not element.no_default_add:
                        dom_element.add_child(element, context)
                else:
                    stream.rewind(checkpoint)
                    return None
        else:
            # If there was no opening brace then this was a forward declaration
            dom_element.is_forward_declaration = True

        context.current_content_parser = old_content_parser

        stream.get_token_of_type(['SEMICOLON'])  # Eat the trailing semicolon

        return dom_element

    @staticmethod
    def parse_content(context, stream):
        # Allow common element types (comments/etc)
        common_element = code_dom.element.DOMElement.parse_common(context, stream)
        if common_element is not None:
            return common_element

        # Eat commas - technically we shouldn't really need to do this but there are some constructs involving
        # #ifdefs inside enums that are hard to parse "correctly" without it
        stream.get_token_of_type(['COMMA'])

        # Anything not a common element type must be an enum element
        element = code_dom.enumelement.DOMEnumElement.parse(context, stream)

        if element is not None:
            return element
        else:
            return None

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.is_enum_class:
            # Namespaced "enum class" enum
            name = self.name
            if leaf_name != "":
                name += "::" + leaf_name
            if self.parent is not None:
                return self.parent.get_fully_qualified_name(name, include_leading_colons)
            else:
                return name
        else:
            # Non-namespaced old-style enum
            if self.parent is not None:
                return ("::" if include_leading_colons else "") + \
                       self.parent.get_fully_qualified_name(self.name, include_leading_colons)
            else:
                return ("::" if include_leading_colons else "") + self.name

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):

        self.write_preceding_comments(file, indent, context)

        if context.for_c:
            # C doesn't support storage types nor forward-declaration of enums
            if self.name is not None and not self.emit_as_anonymous_for_c:
                # Named enums should be typedefs
                write_c_line(file, indent, self.add_attached_comment_to_line("typedef enum "))
            else:
                # Anonymous enums should not be typedefs
                write_c_line(file, indent, self.add_attached_comment_to_line("enum "))
            write_c_line(file, indent, "{")

            # Write enum elements
            for child in self.children:
                child.write_to_c(file, indent + 1, context)

            if self.name is not None and not self.emit_as_anonymous_for_c:
                write_c_line(file, indent, "} " + self.name + ";")
            else:
                write_c_line(file, indent, "};")
        else:
            storage_type_declaration = ""

            if self.storage_type is not None:
                storage_type_declaration = " : " + self.storage_type.to_c_string(context)

            terminator = ""

            if self.is_forward_declaration:
                terminator = ";"

            write_c_line(file, indent, self.add_attached_comment_to_line("enum " +
                                                                         self.name +
                                                                         storage_type_declaration +
                                                                         terminator))

            if not self.is_forward_declaration:
                write_c_line(file, indent, "{")
                for child in self.children:
                    child.write_to_c(file, indent + 1, context)
                write_c_line(file, indent, "};")

    def __str__(self):
        return "Enum: " + self.name
