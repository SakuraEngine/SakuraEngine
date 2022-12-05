from src import code_dom


# This modifier removes namespaces and adds the name as a prefix to all the members inside
# custom_mappings can be used to supply "namespace name -> prefix" mappings to use
def apply(dom_root, custom_mappings=None):
    # Iterate through all namespaces
    for namespace in dom_root.list_all_children_of_type(code_dom.DOMNamespace):

        # Figure out the prefix we're going to use for this namespace
        prefix = namespace.name
        if custom_mappings is not None:
            if prefix in custom_mappings:
                prefix = custom_mappings[prefix]

        # Add prefix to names of all contained children
        for child in namespace.list_directly_contained_children():
            # Most elements just have a "name" property
            if hasattr(child, "name"):
                child.old_name = child.name  # Store the old name in case it is needed later
                child.name = prefix + child.name
            # ...but field declarations can have multiple names
            if hasattr(child, "names"):
                child.old_names = child.names.copy()
                for i in range(0..len(child.names)):
                    child.names[i] = prefix + child.names[i]

        # Remove the namespace element and promote the children into the parent scope
        children = namespace.children.copy()
        namespace.parent.replace_child(namespace, children)
