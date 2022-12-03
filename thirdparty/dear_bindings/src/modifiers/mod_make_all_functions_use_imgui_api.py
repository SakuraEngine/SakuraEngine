from src import code_dom


# This modifier simply sets the "use IMGUI_API" (which will become CIMGUI_API when written out) flag on all functions
def apply(dom_root):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        function.is_imgui_api = True
