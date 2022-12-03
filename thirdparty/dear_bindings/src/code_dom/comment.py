from .common import *
from src import code_dom


# A comment
class DOMComment(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.comment_text = None
        self.is_attached_comment = False
        self.is_preceding_comment = False
        self.alignment = 0  # Column to try to align to when outputting (relative to current indent level),
        #                     only affects attached comments

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        dom_element = DOMComment()
        tok = stream.get_token_of_type(['LINE_COMMENT', 'BLOCK_COMMENT'])
        if tok is None:
            return None
        dom_element.tokens = [tok]
        dom_element.comment_text = tok.value
        # print("Comment: " + dom_element.commentText)

        # If this comment appeared immediately after another element on the same line, attach it
        if tok.type == 'LINE_COMMENT' and context.last_element is not None \
                and not isinstance(context.last_element, DOMComment) \
                and not isinstance(context.last_element, code_dom.blanklines.DOMBlankLines):
            context.last_element.attached_comment = dom_element
            dom_element.is_attached_comment = True
            dom_element.parent = context.last_element
            dom_element.no_default_add = True  # Suppress the normal add behaviour as we have added the element here

        return dom_element

    def to_c_string(self):
        return self.comment_text

    # Generate a comment from a string (string must include // or /* */)
    @staticmethod
    def from_string(comment_text):
        comment = DOMComment()
        comment.comment_text = comment_text
        return comment

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        if context.for_implementation:
            return  # No comments in implementation code
        # Attached/preceding comments are written by their attached element
        if not self.is_attached_comment and not self.is_preceding_comment:
            write_c_line(file, indent, self.comment_text)

    def __str__(self):
        if self.is_attached_comment or self.is_preceding_comment:
            return "Attached/preceding comment: " + self.comment_text
        else:
            return "Comment: " + self.comment_text
