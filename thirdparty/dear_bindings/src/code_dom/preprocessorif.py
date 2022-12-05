from .common import *
from src import code_dom


# A #if or #ifdef block (or #elif inside one)
class DOMPreprocessorIf(code_dom.element.DOMElement):
    def __init__(self):
        super().__init__()
        self.is_ifdef = False
        self.is_elif = False
        self.is_negated = False
        self.is_include_guard = False  # Set externally, indicates if this was an added include guard
        self.expression_tokens = []
        self.else_children = []

    # Parse tokens from the token stream given
    @staticmethod
    def parse(context, stream):
        checkpoint = stream.get_checkpoint()

        initial_token = stream.get_token_of_type(['PPIFDEF', 'PPIFNDEF', 'PPIF', 'PPELIF'])
        if initial_token is None:
            stream.rewind(checkpoint)
            return None

        dom_element = DOMPreprocessorIf()

        dom_element.is_ifdef = (initial_token.type == 'PPIFDEF') or (initial_token.type == 'PPIFNDEF')
        dom_element.is_elif = initial_token.type == 'PPELIF'
        dom_element.is_negated = initial_token.type == 'PPIFNDEF'

        # Tokens up until the line end are part of the expression
        while True:
            token = stream.get_token(skip_newlines=False)
            if token is None:
                break

            if (token.type == 'LINE_COMMENT') or (token.type == 'BLOCK_COMMENT'):
                stream.rewind_one_token()
                dom_element.attached_comment = code_dom.comment.DOMComment.parse(context, stream)
                dom_element.attached_comment.is_attached_comment = True
                dom_element.attached_comment.parent = dom_element
            elif token.type == 'NEWLINE':
                break
            else:
                dom_element.expression_tokens.append(token)

        # Then we have the actual body of the conditional
        in_else = False

        while True:
            # "Peek" not "get" here because we need to parse the elif if it exists
            if stream.peek_token_of_type(['PPELIF']) is not None:
                # We expand elifs into a new if inside the else clause
                elif_clause = DOMPreprocessorIf.parse(context, stream)
                if elif_clause is None:
                    stream.rewind(checkpoint)
                    return None
                if not dom_element.no_default_add:
                    dom_element.add_child_to_else(elif_clause, context)
                break

            if stream.get_token_of_type(['PPELSE']) is not None:
                in_else = True

            if stream.get_token_of_type(['PPENDIF']) is not None:
                break

            child_element = context.current_content_parser()

            if child_element is not None:
                if not child_element.no_default_add:
                    if in_else:
                        dom_element.add_child_to_else(child_element, context)
                    else:
                        dom_element.add_child(child_element, context)
            else:
                print("Unrecognised element " + str(stream.peek_token()))
                break

        return dom_element

    # Returns true if this has the same condition (expression+flags) as another
    def condition_matches(self, other):
        return (self.get_expression() == other.get_expression()) and \
               (self.is_ifdef == other.is_ifdef) and \
               (self.is_elif == other.is_elif) and \
               (self.is_negated == other.is_negated)

    # Returns true if this is mutually exclusive with another condition
    # (i.e. it is impossible for both to be active at the same time)
    # Note that this currently only detects #ifdef/#ifndef pairs, not more complex expressions, nor does it check for
    # the effect of being in the else-side of a conditional
    def condition_is_mutually_exclusive(self, other):
        return (self.get_expression() == other.get_expression()) and \
               (self.is_ifdef == other.is_ifdef) and \
               (self.is_elif == other.is_elif) and \
               (self.is_negated != other.is_negated)

    # Returns true if the element given is part of our else block
    # (see utils.is_in_else_clause for a version of this that works for non-direct-children)
    def is_element_in_else_block(self, element):
        if element in self.else_children:
            return True
        if element in self.children:
            return False
        if element.parent is None:
            return False
        return self.is_element_in_else_block(element.parent)

    # Get the expression used as a string
    def get_expression(self):
        return collapse_tokens_to_string(self.expression_tokens)

    # Get the opening clause as a string
    def get_opening_clause(self):
        if self.is_ifdef:
            if self.is_negated:
                return "#ifndef " + collapse_tokens_to_string(self.expression_tokens)
            else:
                return "#ifdef " + collapse_tokens_to_string(self.expression_tokens)
        else:
            if self.is_negated:
                return "#if !(" + collapse_tokens_to_string(self.expression_tokens) + ")"
            else:
                return "#if " + collapse_tokens_to_string(self.expression_tokens)

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)

        opening_clause = self.get_opening_clause()
        write_c_line(file, 0, opening_clause)

        for child in self.children:
            child.write_to_c(file, indent, context)

        if len(self.else_children) > 0:
            write_c_line(file, 0, "#else")
            for child in self.else_children:
                child.write_to_c(file, indent, context)

        # If we don't have an existing attached comment, note the opening clause
        if self.attached_comment is not None:
            comment = self.attached_comment.to_c_string()
        else:
            comment = " // " + opening_clause

        write_c_line(file, 0, "#endif" + comment)

    # Add a new child to the else list of this element, optionally setting the last element information in the context
    def add_child_to_else(self, child, context=None):
        child.parent = self
        self.else_children.append(child)
        if context is not None:
            context.last_element = child

    # Remove a child from the else list of this element
    def remove_child_from_else(self, child):
        if child.parent is not self:
            raise Exception("Attempt to remove child from element other than parent")
        self.else_children.remove(child)
        child.parent = None

    def __is_preprocessor_container(self):
        return True

    def get_child_lists(self):
        lists = code_dom.element.DOMElement.get_child_lists(self)
        lists.append(self.else_children)
        return lists

    def get_writable_child_lists(self):
        lists = code_dom.element.DOMElement.get_writable_child_lists(self)
        lists.append(self.else_children)
        return lists

    def clone_without_children(self):
        temp_else_children = self.else_children
        self.else_children = []
        clone = code_dom.element.DOMElement.clone_without_children(self)
        self.else_children = temp_else_children
        return clone

    def __str__(self):
        if self.is_ifdef:
            if self.is_negated:
                return "Ifndef: " + collapse_tokens_to_string(self.expression_tokens)
            else:
                return "Ifdef: " + collapse_tokens_to_string(self.expression_tokens)
        else:
            return "If: " + collapse_tokens_to_string(self.expression_tokens)

    # Dump this element for debugging
    def dump(self, indent=0):
        print("".ljust(indent * 4) + str(self))
        print("".ljust((indent + 1) * 4) + "If-block:")
        for child in self.children:
            child.dump(indent + 2)
        if len(self.else_children) > 0:
            print("".ljust((indent + 1) * 4) + "Else-block:")
            for child in self.else_children:
                child.dump(indent + 2)
