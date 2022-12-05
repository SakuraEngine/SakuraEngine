from .common import *
from src import code_dom


# A generic unparsable... something
class DOMUnparsableThing(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['THING'])
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMUnparsableThing()

        dom_element.tokens.append(token)

        stream.get_token_of_type(['SEMICOLON'])  # Eat semicolons after unparsables

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, self.add_attached_comment_to_line(collapse_tokens_to_string(self.tokens)))

    def __str__(self):
        return "Unparsable: " + str(self.tokens)
