from .common import *
from src import code_dom


# An #undef statement
class DOMUndef(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        initial_token = stream.get_token_of_type(['PPUNDEF'])
        if initial_token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMUndef()

        # Tokens up until the line end are part of the expression
        while True:
            token = stream.get_token(skip_newlines=False)
            if token is None:
                break

            if token.type == 'NEWLINE':
                break

            dom_element.tokens.append(token)

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, self.add_attached_comment_to_line("#undef " +
                                                                     collapse_tokens_to_string(self.tokens)))

    def __str__(self):
        return "Undef: " + collapse_tokens_to_string(self.tokens)
