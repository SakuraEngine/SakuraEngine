from .common import *
from src import code_dom


# A function pointer type
class DOMFunctionPointerType(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None
        self.return_type = None
        self.arguments = []
        self.is_cdecl = False

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()
        dom_element = DOMFunctionPointerType()

        # Return type
        # *Technically* there's nothing stopping someone declaring a function pointer that returns a function
        # pointer, but that generates an annoying infinite loop in the parsing here so we'll disallow it for now

        dom_element.return_type = code_dom.type.DOMType.parse(context, stream, allow_function_pointer=False)
        if dom_element.return_type is None:
            stream.rewind(checkpoint)
            return None
        dom_element.return_type.parent = dom_element

        # We expect a bracket and asterisk before the name, possibly with a macro in between

        if stream.get_token_of_type(["LPAREN"]) is None:
            stream.rewind(checkpoint)
            return None

        # This is a very special-case bodge to deal with one single instance that appears in imgui_internal.h
        # (ImQsort). A more generic solution would be nice.
        if stream.peek_token().value == 'IMGUI_CDECL':
            stream.get_token()  # Eat this token
            dom_element.is_cdecl = True

        if stream.get_token_of_type(["ASTERISK"]) is None:
            stream.rewind(checkpoint)
            return None

        # Function name

        name_token = stream.get_token_of_type(["THING"])
        if name_token is None:
            stream.rewind(checkpoint)
            return None
        dom_element.name = name_token.value

        # Closing bracket

        if stream.get_token_of_type(["RPAREN"]) is None:
            stream.rewind(checkpoint)
            return None

        # Arguments

        if stream.get_token_of_type(["LPAREN"]) is None:
            # Not a valid function declaration
            stream.rewind(checkpoint)
            return None

        while True:
            # Check if we've reached the end of the argument list
            if stream.get_token_of_type(['RPAREN']) is not None:
                break

            arg = code_dom.functionargument.DOMFunctionArgument.parse(context, stream)
            if arg is None:
                stream.rewind(checkpoint)
                return None

            arg.parent = dom_element
            dom_element.arguments.append(arg)

            # Eat any trailing comma
            stream.get_token_of_type(["COMMA"])

        # print(dom_element)
        return dom_element

    def get_child_lists(self):
        lists = code_dom.element.DOMElement.get_child_lists(self)
        lists.append(self.arguments)
        if self.return_type is not None:
            lists.append([self.return_type])
        return lists

    def get_writable_child_lists(self):
        lists = code_dom.element.DOMElement.get_writable_child_lists(self)
        lists.append(self.arguments)
        return lists

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(self.name, include_leading_colons)
        else:
            return self.name

    # Returns true if this type is const
    def is_const(self):
        return False  # Not implemented

    def get_primary_type_name(self):
        return "FnPtr"

    def to_c_string(self, context=WriteContext()):
        cdecl_statement = " IMGUI_CDECL " if self.is_cdecl else ""
        result = self.return_type.to_c_string(context) + " (*" + cdecl_statement + str(self.name) + ")("
        if len(self.arguments) > 0:
            first_arg = True
            for arg in self.arguments:
                if not first_arg:
                    result += ", "
                result += arg.to_c_string(context)
                first_arg = False
        result += ")"
        return result

    def __str__(self):
        result = "Function pointer: Return type=" + str(self.return_type) + " Name=" + str(self.name)
        if len(self.arguments) > 0:
            result += " Arguments="
            for arg in self.arguments:
                result += " [" + str(arg) + "]"
        return result
