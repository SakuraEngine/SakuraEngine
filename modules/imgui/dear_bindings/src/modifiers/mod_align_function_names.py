from src import code_dom
from src import utils


# This modifier tries to align function names that appear together where possible (purely for aesthetic purposes)
def apply(dom_root):

    function_groups = []  # Array of group arrays
    grouped_functions = {}  # Elements we have already assigned to groups

    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        if function not in grouped_functions:
            group = []

            # Scan down until we hit a blank line or an already-grouped function
            current = function
            while (current is not None) and \
                    (not isinstance(current, code_dom.DOMBlankLines)) and \
                    (current not in grouped_functions):
                # We add statements here even if they don't have an attached comment themselves, because it looks bad if
                # we have a bunch of interspaced statements that are longer than the comment alignment.
                # But we ignore full-on comments as they tend to be long and don't affect the aesthetics so much.
                if isinstance(current, code_dom.DOMFunctionDeclaration):
                    group.append(current)
                    grouped_functions[current] = True
                current = current.parent.get_next_child(current)

            function_groups.append(group)

    # Now we have our groups, we just need to align everything within them

    for group in function_groups:

        # First calculate the length of the prefixes and return type for each function in the group

        prefix_lengths = []

        # Context for writing - needs to match that used by the actual header file output
        write_context = code_dom.WriteContext()
        write_context.for_c = True

        for function in group:
            length = len(function.get_prefixes_and_return_type(write_context)) - 1  # -1 to remove trailing space
            prefix_lengths.append(length)

        # Calculate the average prefix length

        average = 0
        if len(prefix_lengths) > 0:
            for length in prefix_lengths:
                average += length
            average /= len(prefix_lengths)

        # Calculate the longest prefix length, ignoring any that are more than 20 characters over the average
        # (to avoid a single very long line pushing everything far to the right)

        alignment = 0
        for length in prefix_lengths:
            if length < (average + 20):
                alignment = max(alignment, length + 1)  # +1 to leave a space after the end of the statement

        # Set all functions in the group to align to the same value

        for function in group:
            function.function_name_alignment = alignment
