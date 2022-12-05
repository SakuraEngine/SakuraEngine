from src import code_dom
import os
from src import utils


# This modifier removes "#pragma once" and replaces it with a #define-based include guard
def apply(dom_root):
    for pragma in dom_root.list_all_children_of_type(code_dom.DOMPragma):
        if pragma.get_pragma_text() == "#pragma once":
            # Remove pragma
            pragma.parent.remove_child(pragma)

            # Generate a (hopefully!) suitable name for our guard define from the header filename
            guard_name = os.path.basename(dom_root.filename).upper().replace('.', '_')

            # Add #ifndef check
            ifguard = code_dom.DOMPreprocessorIf()
            ifguard.is_ifdef = True
            ifguard.is_negated = True
            ifguard.expression_tokens = [utils.create_token(guard_name)]

            # Add #define
            define = code_dom.DOMDefine()
            define.name = guard_name

            ifguard.add_child(define)

            # Move all children of the header inside the #ifndef block
            # add_child() removes the element from the existing container so we can't iterate normally
            while len(dom_root.children) > 0:
                ifguard.add_child(dom_root.children[0])

            # Mark this as being an include guard
            ifguard.is_include_guard = True

            # ...and add the #ifndef as the sole child of the header document
            dom_root.add_child(ifguard)
