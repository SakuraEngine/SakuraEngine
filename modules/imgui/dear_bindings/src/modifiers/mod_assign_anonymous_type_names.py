from src import code_dom
from src import utils


# This modifier assigns synthetic names to any anonymous types
# These aren't relevant to the C generator but are used by the JSON metadata
def apply(dom_root):
    index = 0
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        if struct.is_anonymous and struct.name is None:
            struct.name = "<anonymous" + str(index) + ">"
            index += 1
