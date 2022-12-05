from src import code_dom
import os
from src import utils


# This modifier wraps (almost) the whole file in an "extern C" block
def apply(dom_root):
    # We need to do a slightly fiddly thing here because we don't want to wrap "#pragma once" in our extern C block,
    # so if there's a "#pragma once" then only wrap everything after it

    # Find any #pragma once
    pragma_once = None
    for pragma in dom_root.list_all_children_of_type(code_dom.DOMPragma):
        if pragma.get_pragma_text() == "#pragma once":
            pragma_once = pragma
            break

    # Generate our new elements
    elements_to_append = []

    # Add a blank line above the extern C block to be neat
    elements_to_append.append(code_dom.DOMBlankLines(1))

    # Generate a DOM element for our extern C block
    extern_c = code_dom.DOMExternC()
    extern_c.is_cpp_guarded = True

    elements_to_append.append(extern_c)

    # Move everything into it

    children_to_move = []

    if pragma_once is not None:
        # Add everything after the #pragma once
        current = pragma_once.parent.get_next_child(pragma_once)
        while current is not None:
            children_to_move.append(current)
            current = pragma_once.parent.get_next_child(current)
    else:
        # Just add the whole file if there was no #pragma once
        for child in dom_root.children:
            children_to_move.append(child)

    for child in children_to_move:
        extern_c.add_child(child)

    # Add the extern_c block either to the top of the document or after the #pragma once

    if pragma_once is not None:
        pragma_once.parent.insert_after_child(pragma_once, elements_to_append)
    else:
        for element in elements_to_append:
            dom_root.add_child(element)
