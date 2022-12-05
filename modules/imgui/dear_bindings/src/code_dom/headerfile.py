from .common import *
from src import code_dom


# A single header file
class DOMHeaderFile(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.filename = None  # The filename this header came from (or will be saved as)

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        dom_element = DOMHeaderFile()

        # Set up the default context parser
        old_content_parser = context.current_content_parser
        context.current_content_parser = lambda: DOMHeaderFile.parse_content(context, stream)

        while True:
            child_element = context.current_content_parser()

            if child_element is not None:
                if not child_element.no_default_add:
                    dom_element.add_child(child_element, context)
            else:
                break

        dom_element.parse_content(context, stream)

        context.current_content_parser = old_content_parser

        return dom_element

    @staticmethod
    def parse_content(context, stream):
        return code_dom.element.DOMElement.parse_basic(context, stream)

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        for child in self.children:
            child.write_to_c(file, indent=indent, context=context)

    def __str__(self):
        if self.filename is not None:
            return "Header file: " + self.filename
        else:
            return "Header file"
