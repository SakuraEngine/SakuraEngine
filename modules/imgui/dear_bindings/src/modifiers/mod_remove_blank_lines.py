from src import code_dom
from src import utils


# This modifier removes instances of multiple blank lines, or blank lines at the start/end of an element
# (this is purely for aesthetics)
def apply(dom_root):
    elements_to_consider = dom_root.list_all_children_of_type(code_dom.DOMBlankLines)

    for element in elements_to_consider:
        # Cap number of lines
        element.num_blank_lines = min(element.num_blank_lines, 1)

        if (element.parent.get_prev_child(element) is None) or \
                (element.parent.get_next_child(element) is None):
            # Element is first/last in a container, so remove it
            element.parent.remove_child(element)
