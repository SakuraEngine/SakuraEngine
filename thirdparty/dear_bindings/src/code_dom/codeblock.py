from .common import *
from src import code_dom


# A code block
class DOMCodeBlock(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.tokens = []
        self.code_on_different_line_to_braces = False

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()
        dom_element = DOMCodeBlock()

        # Opening brace

        if stream.get_token_of_type(['LBRACE']) is None:
            stream.rewind(checkpoint)
            return None

        # If there's a newline after the brace, record that fact so we can format the block appropriately when
        # writing it
        if stream.get_token_of_type('NEWLINE', skip_newlines=False) is not None:
            dom_element.code_on_different_line_to_braces = True

        # Eat all tokens until a matching closing brace
        brace_count = 1

        while True:
            # We turn off skip_newlines/skip_whitespace here to preserve the original code formatting
            token = stream.get_token(skip_newlines=False, skip_whitespace=False)
            if token.type == 'LBRACE':
                brace_count += 1
            elif token.type == 'RBRACE':
                brace_count -= 1
                if brace_count == 0:
                    break
            dom_element.tokens.append(token)

        return dom_element

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        if self.code_on_different_line_to_braces:
            write_c_line(file, indent, "{")
            write_c_line(file, indent + 1, collapse_tokens_to_string_with_whitespace(self.tokens))
            write_c_line(file, indent, self.add_attached_comment_to_line("};"))
        else:
            write_c_line(file, indent + 1,
                         self.add_attached_comment_to_line("{ " +
                                                           collapse_tokens_to_string_with_whitespace(self.tokens) +
                                                           " }"))

    def __str__(self):
        return "CodeBlock: Length=" + str(len(self.tokens))
