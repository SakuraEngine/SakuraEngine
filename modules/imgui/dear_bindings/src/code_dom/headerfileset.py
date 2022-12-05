from .common import *
from src import code_dom


# A collection of header files
class DOMHeaderFileSet(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        for child in self.children:
            child.write_to_c(file, indent=indent, context=context)

    def __str__(self):
        return "Header file set"
