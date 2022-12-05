from .common import *
from src import code_dom


# A single function argument
class DOMFunctionArgument(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.arg_type = None
        self.name = None  # May be none as arguments can be unnamed
        self.default_value_tokens = None
        self.is_varargs = False
        self.is_array = False
        self.array_bounds = None
        self.is_implicit_default = False  # Set if this argument should not be exposed, but always treated as default
        #                                   (see mod_generate_default_argument_functions)

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()
        dom_element = DOMFunctionArgument()

        # First check for varargs (...) as an argument

        if stream.get_token_of_type(['ELLIPSES']) is not None:
            dom_element.is_varargs = True
            return dom_element

        # Type

        dom_element.arg_type = code_dom.DOMType.parse(context, stream)
        if dom_element.arg_type is None:
            stream.rewind(checkpoint)
            return None
        dom_element.arg_type.parent = dom_element

        if isinstance(dom_element.arg_type, code_dom.DOMFunctionPointerType):
            # Function pointers have a name specified as part of their declaration
            dom_element.name = dom_element.arg_type.name

        # Argument names are optional, so we have to check for that
        arg_name_token = stream.get_token_of_type(["THING", "COMMA", "RBRACKET"])

        if arg_name_token is not None:
            if arg_name_token.type == "COMMA" or arg_name_token.type == "RBRACKET":
                stream.rewind_one_token()
                return dom_element

            dom_element.name = arg_name_token.value

        # Check for an array specifier

        if stream.get_token_of_type(["LSQUARE"]) is not None:
            dom_element.is_array = True
            # Check for bounds (todo: support multiple bounds)
            tok = stream.get_token_of_type(['DECIMAL_LITERAL'])
            if tok is not None:
                dom_element.array_bounds = int(tok.value)
            if stream.get_token_of_type(["RSQUARE"]) is None:
                stream.rewind_one_token()
                return dom_element

        # Check for a default value

        if stream.get_token_of_type("EQUAL"):
            dom_element.default_value_tokens = []

            bracket_count = 1

            while True:
                token = stream.get_token()
                if token.type == "LPAREN":
                    bracket_count += 1
                elif token.type == "RPAREN":
                    bracket_count -= 1
                    if bracket_count == 0:
                        stream.rewind_one_token()
                        break
                elif token.type == "COMMA":
                    if bracket_count == 1:  # Comma at the top level terminates the expression
                        stream.rewind_one_token()
                        break

                dom_element.default_value_tokens.append(token)

        return dom_element

    def get_child_lists(self):
        lists = code_dom.DOMElement.get_child_lists(self)
        if self.arg_type is not None:
            lists.append([self.arg_type])
        return lists

    def get_writable_child_lists(self):
        return code_dom.DOMElement.get_writable_child_lists(self)

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(self.name, include_leading_colons)
        else:
            return self.name

    def get_default_value(self):
        return collapse_tokens_to_string(self.default_value_tokens)

    def to_c_string(self, context=WriteContext()):
        if self.is_varargs:
            return "..."
        result = self.arg_type.to_c_string(context)
        # Don't write the name if this is a function pointer, because the type declaration already includes the name
        if (self.name is not None) and not isinstance(self.arg_type, code_dom.DOMFunctionPointerType):
            result += " " + str(self.name)
        if self.is_array:
            result += "[" + str(self.array_bounds or "") + "]"
        if (self.default_value_tokens is not None) and not context.for_implementation:
            if context.for_c:
                #  C doesn't support default arguments, so just include them as a comment
                result += " /* = " + collapse_tokens_to_string(self.default_value_tokens) + " */"
            else:
                result += " = " + collapse_tokens_to_string(self.default_value_tokens)
        return result

    def __str__(self):
        if self.is_varargs:
            return "Arg: ..."
        result = "Arg: Type=" + str(self.arg_type) + " Name=" + str(self.name)
        if self.is_array:
            result += " (array type)"
        if self.default_value_tokens is not None:
            result += " Default=" + collapse_tokens_to_string(self.default_value_tokens)
        if self.is_implicit_default:
            result += " (implicit default)"
        return result
