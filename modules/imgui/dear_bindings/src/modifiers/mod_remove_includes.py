from src import code_dom
from src import utils


# This modifier removes the listed #includes
# include_filenames should be a list of filenames in include syntax (e.g. "<stdio.h>" or similar)
def apply(dom_root, include_filenames):
    for include in dom_root.list_all_children_of_type(code_dom.DOMInclude):
        if include.get_include_file_name() in include_filenames:
            include.parent.remove_child(include)
