from .common import *
from src import code_dom


# A #define statement
class DOMDefine(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None  # The name of the define
        self.content = None  # The actual content of the define (None if it is just a basic #define)

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['PPDEFINE'])
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMDefine()

        dom_element.tokens = [token]

        # Due to the way the main parser handles defines we do a little bit of special-case parsing here
        content = token.value[len('#define '):]
        content_bits = content.split(sep=None, maxsplit=1)

        dom_element.name = content_bits[0].strip()
        if len(content_bits) > 1:
            dom_element.content = content_bits[1].strip()

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        # This is a little bit weird because we want to try and preserve formatting (whitespace), which means if we
        # have tokens we use those to generate the output, otherwise we synthesize a new statement
        if len(self.tokens) > 0:
            write_c_line(file, indent, self.add_attached_comment_to_line(collapse_tokens_to_string(self.tokens)))
        elif self.content is not None:
            write_c_line(file, indent, self.add_attached_comment_to_line("#define " + self.name + " " + self.content))
        else:
            write_c_line(file, indent, self.add_attached_comment_to_line("#define " + self.name))

    def __str__(self):
        return "Define: " + str(self.tokens)
