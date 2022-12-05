from src import code_dom
from src import utils


# This modifier tries to align enum values that appear together where possible (purely for aesthetic purposes)
def apply(dom_root):
    for enum in dom_root.list_all_children_of_type(code_dom.DOMEnum):
        # Calculate the maximum name length within the enum
        max_name_length = 0
        for enum_element in enum.list_all_children_of_type(code_dom.DOMEnumElement):
            max_name_length = max(max_name_length, len(enum_element.name))

        # Set all the enum items to pad to that length
        for enum_element in enum.list_all_children_of_type(code_dom.DOMEnumElement):
            enum_element.value_alignment = max_name_length
            # utils.append_comment_text(enum_element, " Value align = " + str(max_name_length))
