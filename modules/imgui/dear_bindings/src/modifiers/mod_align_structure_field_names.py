from src import code_dom
from src import utils


# This modifier tries to align field names in structures where possible (purely for aesthetic purposes)
def apply(dom_root):
    for enum in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        # Calculate the prefix lengths for all fields in the struct
        prefix_lengths = []

        # Context for writing - needs to match that used by the actual header file output
        write_context = code_dom.WriteContext()
        write_context.for_c = True

        for field in enum.list_all_children_of_type(code_dom.DOMFieldDeclaration):
            prefix_lengths.append(len(field.get_prefix_and_type(write_context)))

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
                alignment = max(alignment, length)

        # Set all the names to align to that
        for field in enum.list_all_children_of_type(code_dom.DOMFieldDeclaration):
            field.name_alignment = alignment
            # utils.append_comment_text(field, " Name align = " + str(alignment))
