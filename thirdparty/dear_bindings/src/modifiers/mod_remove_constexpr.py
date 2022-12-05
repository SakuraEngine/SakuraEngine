from src import code_dom


# This modifier removes constexpr from everything in the DOM that has it
def apply(dom_root):

    # First functions
    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        function.is_constexpr = False

    # Then any constexpr tokens in types
    for domType in dom_root.list_all_children_of_type(code_dom.DOMType):
        if domType.is_constexpr():
            new_tokens = []
            for token in domType.tokens:
                if token.type != 'CONSTEXPR':
                    new_tokens.append(token)
            domType.tokens = new_tokens
