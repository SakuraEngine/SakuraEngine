from .common import *
from src import code_dom


# A C++ template
class DOMTemplate(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.template_parameter_tokens = []

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        if stream.get_token_of_type(['TEMPLATE']) is None:
            stream.rewind(checkpoint)
            return

        dom_element = DOMTemplate()

        if stream.get_token_of_type(['LTRIANGLE']) is None:
            stream.rewind(checkpoint)
            return

        # Now we expect a list of parameters, terminated with a >

        brace_count = 1

        while True:
            token = stream.get_token()
            if token.type == 'LTRIANGLE':
                brace_count += 1
            elif token.type == 'RTRIANGLE':
                brace_count -= 1
                if brace_count == 0:
                    break
            dom_element.template_parameter_tokens.append(token)

        # The next thing will either be a struct or a function

        while True:
            # Allow common element types (comments/etc)
            common_element = code_dom.DOMElement.parse_common(context, stream)
            if common_element is not None:
                if not common_element.no_default_add:
                    dom_element.add_child(common_element, context)
            else:
                tok = stream.peek_token()

                if (tok.type == 'STRUCT') or (tok.type == 'CLASS'):
                    element = code_dom.DOMClassStructUnion.parse(context, stream)

                    if element is None:
                        stream.rewind(checkpoint)
                        return None

                    if not element.no_default_add:
                        dom_element.add_child(element, context)
                else:
                    element = code_dom.DOMFunctionDeclaration.parse(context, stream)

                    if element is None:
                        stream.rewind(checkpoint)
                        return None

                    if not element.no_default_add:
                        dom_element.add_child(element, context)

                # template<> only has a single (non-comment-like) child, so stop here
                break

        return dom_element

    # Get the class/function this template is for
    def get_templated_object(self):
        for child in self.children:
            if isinstance(child, code_dom.DOMClassStructUnion) or isinstance(child, code_dom.DOMFunctionDeclaration):
                return child
        return None

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent,
                     self.add_attached_comment_to_line("template <" +
                                                       collapse_tokens_to_string(self.template_parameter_tokens) + ">"))
        for child in self.children:
            child.write_to_c(file, indent, context)

    def __str__(self):
        return "Template: " + collapse_tokens_to_string(self.template_parameter_tokens)
