from src import code_dom
from src import utils


# This modifier tries to align attached comments that appear together where possible (purely for aesthetic purposes)
def apply(dom_root):
    # First generate groups of comments from structures/enums, as we want to be sure those are grouped together

    comment_groups = []  # Array of group arrays
    grouped_elements = {}  # Elements we have already assigned to groups

    structure_elements = []

    structure_elements.extend(dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion))
    structure_elements.extend(dom_root.list_all_children_of_type(code_dom.DOMEnum))

    for struct in structure_elements:
        # Don't try and do anything with forward declarations at this point, as they don't have any children
        # and trying to be clever here just impairs our ability to nicely align blocks of "typedef struct" statements
        # (which the loose element scan below will do just fine)
        if isinstance(struct, code_dom.DOMClassStructUnion) and struct.is_forward_declaration:
            continue

        group = []

        if struct.attached_comment is not None:
            # Count the struct itself as part of the group if it has a comment
            group.append(struct)
            grouped_elements[struct] = True

        def walker(walker_element):
            if walker_element.attached_comment is not None:
                group.append(walker_element)
                grouped_elements[walker_element] = True

        struct.walk(walker)

        comment_groups.append(group)

    # Next look for any other elements with comments that seem interesting and group them according to their
    # position in the file

    for element in dom_root.list_all_children_of_type(code_dom.DOMElement):
        if (element.attached_comment is not None) and (element not in grouped_elements):
            group = []

            # Scan down until we hit a blank line or an already-grouped element
            current = element
            while (current is not None) and \
                    (not isinstance(current, code_dom.DOMBlankLines)) and \
                    (current not in grouped_elements):
                # We add statements here even if they don't have an attached comment themselves, because it looks bad if
                # we have a bunch of interspaced statements that are longer than the comment alignment.
                # But we ignore full-on comments as they tend to be long and don't affect the aesthetics so much.
                if not isinstance(current, code_dom.DOMComment):
                    group.append(current)
                    grouped_elements[current] = True
                current = current.parent.get_next_child(current)

            comment_groups.append(group)

    # Now we have our groups, we just need to align everything within them

    for group in comment_groups:

        # First calculate the length of each statement in the group

        statement_lengths = []

        # A fake file we get the DOM to write the C code to so we can evaluate the length
        class FakeFile:
            def __init__(self):
                self.max_length = 0

            def write(self, line):
                self.max_length = max(self.max_length, len(line))

        # Context for writing - needs to match that used by the actual header file output
        write_context = code_dom.WriteContext()
        write_context.for_c = True

        for element in group:
            # Temporarily remove any comments so we can get the actual statement length without them
            comment = element.attached_comment
            pre_comments = element.pre_comments
            element.attached_comment = None
            element.pre_comments = []

            # Write to fake file and extract maximum length
            file = FakeFile()
            element.write_to_c(file, 0, write_context)
            statement_lengths.append(file.max_length)

            # Reattach comments
            element.attached_comment = comment
            element.pre_comments = pre_comments
            #  if element.attached_comment:
            #    element.attached_comment.comment_text += " (statement length " + str(file.max_length) + ")"

        # Calculate the average statement length

        average = 0
        if len(statement_lengths) > 0:
            for length in statement_lengths:
                average += length
            average /= len(statement_lengths)

        # Calculate the longest statement length, ignoring any that are more than 40 characters over the average
        # (to avoid a single very long line pushing everything far to the right)

        alignment = 0
        for length in statement_lengths:
            if length < (average + 40):
                alignment = max(alignment, length + 1)  # +1 to leave a space after the end of the statement

        # Set all elements in the group to align to the same value

        for element in group:
            if element.attached_comment is not None:
                element.attached_comment.alignment = alignment

