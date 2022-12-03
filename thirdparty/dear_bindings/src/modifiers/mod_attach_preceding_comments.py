from src import code_dom
from src import utils


# This modifier looks for comments that precede an element and attaches them so they get manipulated with it
def apply(dom_root):

    elements_to_consider = []

    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMFieldDeclaration))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMTypedef))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMTemplate))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMPreprocessorIf))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMEnum))
    elements_to_consider.extend(dom_root.list_all_children_of_type(code_dom.DOMEnumElement))

    for element in elements_to_consider:
        comments = []

        # Walk backwards from the element looking for comments immediately above
        comment_element = element.parent.get_prev_child(element)
        while isinstance(comment_element, code_dom.DOMComment) and not comment_element.is_attached_comment \
                and not comment_element.is_preceding_comment:
            comments.append(comment_element)
            comment_element = comment_element.parent.get_prev_child(comment_element)

        if len(comments) > 0:
            # Reverse the order of the comments as we collected them backwards
            comments.reverse()

            # Add them to the element as preceding comments
            element.attach_preceding_comments(comments)
