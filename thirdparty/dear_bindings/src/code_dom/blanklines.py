from .common import *
from src import code_dom


# A blank line
class DOMBlankLines(code_dom.element.DOMElement):
    def __init__(self, num_lines=0):
        super().__init__()
        self.num_blank_lines = num_lines

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['NEWLINE'], skip_newlines=False)
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMBlankLines()

        # Eat as many newlines as exist

        while stream.get_token_of_type(['NEWLINE'], skip_newlines=False):
            dom_element.num_blank_lines += 1

        dom_element.tokens.append(token)

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        if self.num_blank_lines > 0:
            for i in range(0, self.num_blank_lines):
                write_c_line(file, 0, "")

    def __str__(self):
        return "Blank lines: " + str(self.num_blank_lines)

