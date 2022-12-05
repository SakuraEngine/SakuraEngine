from .common import *
from src import code_dom
import copy


# A type, represented by a sequence of tokens that define it
class DOMType(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream, allow_function_pointer=True):

        if allow_function_pointer:
            # Types may be a function pointer, so check for that first

            dom_element = code_dom.functionpointertype.DOMFunctionPointerType.parse(context, stream)
            if dom_element is not None:
                return dom_element

        # If it wasn't a function pointer, it's probably a normal type

        checkpoint = stream.get_checkpoint()
        dom_element = DOMType()
        have_valid_type = False
        while True:
            tok = stream.get_token_of_type(['THING', 'ASTERISK', 'AMPERSAND', 'CONST', 'CONSTEXPR', 'SIGNED',
                                            'UNSIGNED', 'LSQUARE', 'LTRIANGLE', 'COLON'])
            if tok is None:
                if not have_valid_type:
                    stream.rewind(checkpoint)
                    return None
                else:
                    # We have a valid type
                    return dom_element

            if have_valid_type:
                if tok.type == 'LSQUARE':
                    # Array indicator
                    dom_element.tokens.append(tok)
                    # Check for bounds (todo: support multiple bounds)
                    tok = stream.get_token_of_type(['DECIMAL_LITERAL'])
                    if tok is not None:
                        dom_element.tokens.append(tok)
                    tok = stream.get_token_of_type(['RSQUARE'])
                    if tok is None:
                        # Malformed array bounds
                        print("Expected ] to terminate array bounds")
                        stream.rewind(checkpoint)
                        return None
                    dom_element.tokens.append(tok)
                elif tok.type == 'LTRIANGLE':
                    # Template parameters
                    dom_element.tokens.append(tok)
                    brace_count = 1

                    while True:
                        token = stream.get_token()
                        dom_element.tokens.append(token)
                        if token.type == 'LTRIANGLE':
                            brace_count += 1
                        elif token.type == 'RTRIANGLE':
                            brace_count -= 1
                            if brace_count == 0:
                                break
                elif (tok.type == 'ASTERISK') or (tok.type == 'AMPERSAND') or (tok.type == 'CONST'):
                    # Type suffix
                    dom_element.tokens.append(tok)
                elif tok.type == 'COLON':
                    # Namespace separator
                    dom_element.tokens.append(tok)
                    have_valid_type = False  # Go back to the state of expecting a type name
                elif tok.value == 'long':
                    # "long long" is... a thing :-(
                    dom_element.tokens.append(tok)
                else:
                    # Something else, so stop here
                    stream.rewind_one_token()
                    return dom_element
            else:
                if (tok.type == 'CONST') or (tok.type == 'CONSTEXPR') or (tok.type == 'SIGNED') or \
                        (tok.type == 'UNSIGNED'):
                    # Type prefix
                    dom_element.tokens.append(tok)
                elif tok.type == 'COLON':
                    # Leading namespace separator
                    dom_element.tokens.append(tok)
                else:
                    # Type name
                    dom_element.tokens.append(tok)
                    have_valid_type = True

    # Returns true if this type is const
    # (very conservative - considers the type const if const appears anywhere in it)
    def is_const(self):
        for tok in self.tokens:
            if tok.type == 'CONST':
                return True
        return False

    # Returns true if this type is constexpr
    # (very conservative - considers the type constexpr if constexpr appears anywhere in it)
    def is_constexpr(self):
        for tok in self.tokens:
            if tok.type == 'CONSTEXPR':
                return True
        return False

    # Gets the "primary" type name involved (i.e. without any prefixes or suffixes)
    # This is mostly useful for trying to construct overload disambiguation suffixes
    def get_primary_type_name(self):
        primary_name = None
        triangle_bracket_count = 0
        for tok in self.tokens:
            if (tok.type == 'ASTERISK') or (tok.type == 'AMPERSAND') or (tok.type == 'CONST') \
                    or (tok.type == 'SIGNED') or (tok.type == 'UNSIGNED'):
                continue  # These are never the type name
            elif tok.type == 'LTRIANGLE':
                triangle_bracket_count += 1
            elif tok.type == 'RTRIANGLE':
                triangle_bracket_count -= 1

            if triangle_bracket_count == 0:
                primary_name = tok.value  # We use the last of the "name-like" things we see
        return primary_name

    # Gets the fully-qualified name (C++-style) of this element (including namespaces/etc)
    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        context = WriteContext()
        context.include_leading_colons = include_leading_colons
        return self.to_c_string(context)

    def to_c_string(self, context=WriteContext()):
        # Return the original name if requested
        if context.use_original_names:
            if self.original_name_override is not None:
                return self.original_name_override
            elif self.unmodified_element is not None:
                return self.unmodified_element.to_c_string(context)

        if context.include_leading_colons:
            # Add leading colons to anything that looks like a user type
            fudged_tokens = []
            for tok in self.tokens:
                new_tok = copy.deepcopy(tok)
                if new_tok.type == 'THING':
                    new_tok.value = "::" + new_tok.value
                fudged_tokens.append(new_tok)
            return collapse_tokens_to_string(fudged_tokens)
        else:
            return collapse_tokens_to_string(self.tokens)

    def __str__(self):
        result = "Type: " + collapse_tokens_to_string(self.tokens)
        if self.original_name_override is not None:
            result += " (original name " + self.original_name_override + ")"
        return result
