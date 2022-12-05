from .common import *
from src import code_dom


# Namespace
class DOMNamespace(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        token = stream.get_token_of_type(['NAMESPACE'])
        if token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMNamespace()

        name_token = stream.get_token()
        if name_token is None:
            stream.rewind(checkpoint)
            return None

        dom_element.name = name_token.value

        if stream.get_token_of_type(['LBRACE']) is None:
            stream.rewind(checkpoint)
            return None

        while True:
            tok = stream.peek_token()
            if tok.type == 'RBRACE':
                stream.get_token()  # Eat the closing brace
                stream.get_token_of_type(['SEMICOLON'])  # Eat the trailing semicolon too
                break

            child_element = context.current_content_parser()

            if child_element is not None:
                if not child_element.no_default_add:
                    dom_element.add_child(child_element, context)
            else:
                print("Unrecognised element: " + str(vars(tok)))
                break

        return dom_element

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        name = self.name
        if leaf_name != "":
            name += "::" + leaf_name
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(name, include_leading_colons)
        else:
            return ("::" if include_leading_colons else "") + name

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, self.add_attached_comment_to_line("namespace " + self.name))
        write_c_line(file, indent, "{")
        for child in self.children:
            child.write_to_c(file, indent + 1, context)
        write_c_line(file, indent, "}")

    def __str__(self):
        return "Namespace: " + self.name
