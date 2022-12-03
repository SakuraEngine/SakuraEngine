from src import code_dom


# This modifier takes any nested classes/structs and moves them up to the enclosing scope
def apply(dom_root):
    # Iterate through all structs/classes/unions
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        # Ignore unions
        if struct.structure_type == "UNION":
            continue

        # Is this struct nested?
        parent_struct = struct.get_parent_class()
        if parent_struct is None:
            continue  # Not nested

        # Append the parent struct name to the child name
        new_name = parent_struct.name + "_" + struct.name

        # Find any references to the struct and rename them

        # The original qualified struct name (note: not fully-qualified, only relative to the parent scope)
        qualified_name = parent_struct.name + "::" + struct.name

        # Local-scope references within the parent
        for type_element in parent_struct.list_all_children_of_type(code_dom.DOMType):
            found_element_to_change = False
            for i in range(0, len(type_element.tokens)):
                if type_element.tokens[i].value == struct.name:
                    type_element.tokens[i].value = new_name
                    found_element_to_change = True

            if found_element_to_change:
                # This is a little fiddly - if we changed something, we need to update the original override
                # to use the original name in fully-qualified form, since post-flattening things won't be in the same
                # scope any more
                if type_element.original_name_override is not None:
                    # In this case we don't replace new_name because it won't have been modified by the code above
                    type_element.original_name_override = type_element.original_name_override\
                        .replace(struct.name, qualified_name)
                else:
                    write_context = code_dom.WriteContext()
                    write_context.use_original_names = True
                    type_element.original_name_override = type_element.to_c_string(write_context)\
                        .replace(new_name, qualified_name)

        # Global-scope references using the parent-qualified name
        for type_element in dom_root.list_all_children_of_type(code_dom.DOMType):
            found_element_to_change = False
            for i in range(0, len(type_element.tokens)):
                if type_element.tokens[i].value == qualified_name:
                    type_element.tokens[i].value = new_name

            if found_element_to_change:
                if type_element.original_name_override is None:
                    write_context = code_dom.WriteContext()
                    write_context.use_original_names = True
                    type_element.original_name_override = type_element.to_c_string(write_context)\
                        .replace(new_name, qualified_name)

        # Update the name
        struct.name = new_name

        # Move the structure out into the scope of the parent
        parent_struct.parent.insert_before_child(parent_struct, [struct])
