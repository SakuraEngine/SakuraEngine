from .common import *
from src import code_dom


# A #include
class DOMInclude(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['PPINCLUDE'])
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMInclude()

        dom_element.tokens = [token]

        # Tokens up until the line end or line comment are part of the directive
        while True:
            token = stream.get_token(skip_newlines=False)
            if token is None:
                break

            if (token.type == 'NEWLINE') or (token.type == 'LINE_COMMENT'):
                break

            dom_element.tokens.append(token)

        return dom_element

    # Get the referred include file
    def get_include_file_name(self):
        # Skip token 0 as that is the #include itself
        return collapse_tokens_to_string(self.tokens[1:])

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, self.add_attached_comment_to_line(collapse_tokens_to_string(self.tokens)))

    def __str__(self):
        return "Include: " + str(self.tokens)
