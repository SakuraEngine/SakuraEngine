from src import code_dom


# This modifier renames defines where they appears (note that this does nothing with regards to defines used
# in actual code at the moment, and is really intended for simple "#ifdef BLAH" type use-cases)
# This also attempts to rename any occurrences of the define in comments attached to any define that was altered.
# Takes a map mapping old names to new names
def apply(dom_root, name_map):

    comments_to_check = []

    # Rename in any #defines
    for define in dom_root.list_all_children_of_type(code_dom.DOMDefine):
        for token in define.tokens:
            did_anything = False
            for old_name in name_map:
                if old_name in token.value:
                    token.value = token.value.replace(old_name, name_map[old_name])
                    did_anything = True

            if did_anything:
                if define.pre_comments is not None:
                    comments_to_check.extend(define.pre_comments)
                if define.attached_comment is not None:
                    comments_to_check.append(define.attached_comment)

    # Rename in any conditional expressions
    for conditional in dom_root.list_all_children_of_type(code_dom.DOMPreprocessorIf):
        did_anything = False
        for token in conditional.expression_tokens:
            if token.value in name_map:
                token.value = name_map[token.value]
                did_anything = True

        if did_anything:
            if conditional.pre_comments is not None:
                comments_to_check.extend(conditional.pre_comments)
            if conditional.attached_comment is not None:
                comments_to_check.append(conditional.attached_comment)

    # Update comments on things we altered

    for comment in comments_to_check:
        for old_name in name_map:
            if old_name in comment.comment_text:
                comment.comment_text = comment.comment_text.replace(old_name, name_map[old_name])
