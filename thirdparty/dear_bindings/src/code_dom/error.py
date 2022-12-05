from .common import *
from src import code_dom


# A #error statement
class DOMError(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['PPERROR'])
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMError()

        # The error token contains the entire error body, so we don't need to do much here

        dom_element.tokens = [token]

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, self.add_attached_comment_to_line(collapse_tokens_to_string(self.tokens)))

    def __str__(self):
        return "Error: " + str(self.tokens)
