from src import code_dom
from src import utils


# This modifier finds anywhere that has ended up with adjacent blank line elements and merges them
def apply(dom_root):

    elements_to_consider = dom_root.list_all_children_of_type(code_dom.DOMBlankLines)

    for element in elements_to_consider:
        if element.parent is None:
            continue  # Element has been removed

        surrounding_lines = []

        # Walk backwards from the element looking for more blank lines
        walk_element = element.parent.get_prev_child(element)
        while isinstance(walk_element, code_dom.DOMBlankLines):
            surrounding_lines.append(walk_element)
            walk_element = walk_element.parent.get_prev_child(walk_element)

        # Walk forward from the element looking for more blank lines
        walk_element = element.parent.get_next_child(element)
        while isinstance(walk_element, code_dom.DOMBlankLines):
            surrounding_lines.append(walk_element)
            walk_element = walk_element.parent.get_next_child(walk_element)

        # Merge lines into this one
        for line in surrounding_lines:
            line.parent.remove_child(line)
            element.num_blank_lines += line.num_blank_lines
