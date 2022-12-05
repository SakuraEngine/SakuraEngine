from .common import *
from src import code_dom


# A typedef statement
class DOMTypedef(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.name = None
        self.type = None

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        if stream.get_token_of_type(['TYPEDEF']) is None:
            return None

        dom_element = DOMTypedef()

        dom_element.type = code_dom.type.DOMType.parse(context, stream)
        if dom_element.type is None:
            stream.rewind(checkpoint)
            return None
        dom_element.type.parent = dom_element

        if isinstance(dom_element.type, code_dom.functionpointertype.DOMFunctionPointerType):
            # Function pointers have the name as part of the declaration
            dom_element.name = dom_element.type.name
        else:
            name_tok = stream.get_token_of_type(['THING'])
            if name_tok is None:
                stream.rewind(checkpoint)
                return None
            dom_element.name = name_tok.value

        stream.get_token_of_type(['SEMICOLON'])  # Eat the trailing semicolon

        return dom_element

    def get_child_lists(self):
        lists = code_dom.element.DOMElement.get_child_lists(self)
        if self.type is not None:
            lists.append([self.type])
        return lists

    def get_writable_child_lists(self):
        return code_dom.element.DOMElement.get_writable_child_lists(self)

    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(self.name, include_leading_colons)
        else:
            return self.name

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)

        # Function pointers have the name/etc included
        if isinstance(self.type, code_dom.functionpointertype.DOMFunctionPointerType):
            write_c_line(file, indent, self.add_attached_comment_to_line("typedef " + self.type.to_c_string() + ";"))
        else:
            write_c_line(file, indent, self.add_attached_comment_to_line("typedef " + self.type.to_c_string() +
                                                                         " " + self.name + ";"))

    def __str__(self):
        return "Typedef: " + self.name + " type=" + str(self.type)
