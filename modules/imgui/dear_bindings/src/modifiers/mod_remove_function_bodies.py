from src import code_dom


# This modifier removes any function bodies. Inline functions are set to be IMGUI_API and the inline modifier removed.
def apply(dom_root):
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        function.body = None
        if function.is_inline or function.is_static:
            function.is_inline = False
            function.is_static = False
            function.is_imgui_api = True
