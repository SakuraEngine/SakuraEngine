from src import code_dom


# This modifier removes all references and turns them into pointers or straight pass-by-value
def apply(dom_root):
    for type_element in dom_root.list_all_children_of_type(code_dom.DOMType):

        is_argument = isinstance(type_element.parent, code_dom.DOMFunctionArgument)

        # For function arguments, if the argument is of the form "const X&", then convert it to just "X"
        if is_argument:
            if len(type_element.tokens) == 3:
                if (type_element.tokens[0].type == 'CONST') and (type_element.tokens[2].type == 'AMPERSAND'):
                    type_element.tokens = [type_element.tokens[1]]
                    # We don't set was_reference here because from the code generator's perspective no adjustment
                    # is necessary to turn a value into a reference

        # Find all references and convert them to pointers
        for tok in type_element.tokens:
            if tok.type == 'AMPERSAND':
                # We need to convert this to use a pointer
                tok.type = 'ASTERISK'
                tok.value = '*'
                # Note that we adjusted this so the function stub generator knows it started as a reference
                tok.was_reference = True
