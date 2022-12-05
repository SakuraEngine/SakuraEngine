from .common import *
from src import code_dom


# A single element within an enum
class DOMEnumElement(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None
        self.value_tokens = None
        self.value_alignment = 0  # Column to align values to (for aesthetic purposes)

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        dom_element = DOMEnumElement()

        # It's possible to have a comment here in very rare cases that we don't have a good way to deal with, so
        # for now eat it
        stream.get_token_of_type(['LINE_COMMENT', 'BLOCK_COMMENT'])

        name_tok = stream.get_token_of_type(['THING'])
        if name_tok is None:
            stream.rewind(checkpoint)
            return None

        dom_element.name = name_tok.value

        if stream.get_token_of_type(['EQUAL']) is not None:
            # We have a value
            dom_element.value_tokens = []
            while True:
                tok = stream.get_token(skip_newlines=False)
                # This is a fudge - enum elements can span multiple lines, but we have cases where the comma is inside
                # a #ifdef block on a new line, which causes chaos if we don't break out of the item parser before then
                if tok.type == 'NEWLINE':
                    stream.rewind_one_token(skip_newlines=False)
                    break
                # The same fudge as above is necessary for line comments
                if tok.type == 'LINE_COMMENT':
                    stream.rewind_one_token(skip_newlines=False)
                    break
                if tok.type == 'RBRACE':
                    stream.rewind_one_token(skip_newlines=False)  # Leave the brace for the enum itself to parse
                    break
                if tok.type == 'COMMA':
                    # We're going to eat this in a second, but we don't want to accidentally eat two commas
                    stream.rewind_one_token(skip_newlines=False)
                    break

                dom_element.value_tokens.append(tok)

        stream.get_token_of_type(['COMMA'])  # Eat any trailing comma
        return dom_element

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(self.name, include_leading_colons)
        else:
            return self.name

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        if self.value_tokens is not None:
            # Generate padded version of name to align value
            name_padded = self.name
            if self.value_alignment > len(self.name):
                name_padded = name_padded + (" " * (self.value_alignment - len(self.name)))
            write_c_line(file, indent, self.add_attached_comment_to_line(name_padded + " = " +
                                                                         collapse_tokens_to_string(self.value_tokens) +
                                                                         ","))
        else:
            write_c_line(file, indent, self.add_attached_comment_to_line(self.name + ","))

    def __str__(self):
        if self.value_tokens is None:
            return "EnumElement: " + self.name
        else:
            return "EnumElement: " + self.name + " Value:" + collapse_tokens_to_string(self.value_tokens)
