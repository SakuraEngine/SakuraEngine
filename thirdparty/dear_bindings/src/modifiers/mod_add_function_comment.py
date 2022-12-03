from src import code_dom
from src import utils


# This modifier adds a comment to the function with the fully-qualified name given
# If a comment already exists the comment text will be appended to it with a space
def apply(dom_root, function_name, comment):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if function.get_fully_qualified_name(return_fqn_even_for_member_functions=True) == function_name:
            utils.append_comment_text(function, comment)
