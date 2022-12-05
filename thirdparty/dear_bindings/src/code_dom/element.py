from .common import *
import copy
import src.code_dom


# Base class for all DOM elements
class DOMElement:
    def __init__(self):
        self.tokens = []
        self.parent = None  # The parent element
        self.children = []  # Basic child elements (note that some elements have multiple child lists)
        self.pre_comments = []  # If this element is preceded with comments that are related to it, they go here
        self.attached_comment = None  # If a comment appears after this element (on the same line), this is it
        self.no_default_add = False  # Should this element not be added to the DOM upon creation? (mainly for
        #                              attached comments)
        self.unmodified_element = None  # The original (unmodified) element, as part of a complete clone of the
        #                                 pre-modification document structure
        self.original_name_override = None  # Optional name to use for the original name of this type
        #                                     (primarily for template parameter expansion and the like)
        self.is_internal = False  # Indicates that the associated element is an internal API component
        self.exclude_from_metadata = False  # Should this element be excluded from the generated metadata?

    # Parse tokens that can appear anywhere, returning an appropriate element if possible or None if not
    @staticmethod
    def parse_common(context, stream):
        tok = stream.peek_token(skip_newlines=False)
        if tok is None:
            return None
        if (tok.type == 'LINE_COMMENT') or (tok.type == 'BLOCK_COMMENT'):
            return src.code_dom.comment.DOMComment.parse(context, stream)
        elif tok.type == 'PPDEFINE':
            return src.code_dom.define.DOMDefine.parse(context, stream)
        elif tok.type == 'PPUNDEF':
            return src.code_dom.undef.DOMUndef.parse(context, stream)
        elif (tok.type == 'PPIF') or (tok.type == 'PPIFDEF') or (tok.type == 'PPIFNDEF'):
            return src.code_dom.preprocessorif.DOMPreprocessorIf.parse(context, stream)
        elif tok.type == 'PRAGMA':
            return src.code_dom.pragma.DOMPragma.parse(context, stream)
        elif tok.type == 'PPERROR':
            return src.code_dom.error.DOMError.parse(context, stream)
        elif tok.type == 'PPINCLUDE':
            return src.code_dom.include.DOMInclude.parse(context, stream)
        elif tok.type == 'NEWLINE':
            blank_lines = src.code_dom.blanklines.DOMBlankLines.parse(context, stream)
            # A little bit of a convenience hack here - we don't really want tons of "zero blank lines"
            # entries cluttering up the DOM every time we see a newline, so only return blank line elements if they
            # actually represent a blank line as opposed to just a single newline
            if blank_lines.num_blank_lines > 0:
                return blank_lines
            else:
                context.last_element = None  # Clear last_element to avoid comments attaching across newlines
                return DOMElement.parse_common(context, stream)
        else:
            return None

    # Parse tokens that can appear in most scopes, returning an appropriate element if possible or None if not
    @staticmethod
    def parse_basic(context, stream):
        common_element = DOMElement.parse_common(context, stream)
        if common_element is not None:
            return common_element

        tok = stream.peek_token()
        if tok is None:
            return None
        if (tok.type == 'STRUCT') or (tok.type == 'CLASS') or (tok.type == 'UNION'):
            return src.code_dom.classstructunion.DOMClassStructUnion.parse(context, stream)
        elif tok.type == 'PPDEFINE':
            return src.code_dom.define.DOMDefine.parse(context, stream)
        elif tok.type == 'PPUNDEF':
            return src.code_dom.undef.DOMUndef.parse(context, stream)
        elif (tok.type == 'PPIF') or (tok.type == 'PPIFDEF') or (tok.type == 'PPIFNDEF'):
            return src.code_dom.preprocessorif.DOMPreprocessorIf.parse(context, stream)
        elif tok.type == 'PRAGMA':
            return src.code_dom.pragma.DOMPragma.parse(context, stream)
        elif tok.type == 'PPERROR':
            return src.code_dom.error.DOMError.parse(context, stream)
        elif tok.type == 'PPINCLUDE':
            return src.code_dom.include.DOMInclude.parse(context, stream)
        elif tok.type == 'NAMESPACE':
            return src.code_dom.namespace.DOMNamespace.parse(context, stream)
        elif tok.type == 'TYPEDEF':
            return src.code_dom.typedef.DOMTypedef.parse(context, stream)
        elif tok.type == 'ENUM':
            return src.code_dom.enum.DOMEnum.parse(context, stream)
        elif tok.type == 'TEMPLATE':
            return src.code_dom.template.DOMTemplate.parse(context, stream)
        elif (tok.type == 'THING') or (tok.type == 'CONST') or (tok.type == 'CONSTEXPR') or (tok.type == 'SIGNED') or \
                (tok.type == 'UNSIGNED') or \
                (tok.type == '~'):  # ~ is necessary because destructor names start with it

            # It might be an extern "C" statement

            if tok.value == 'extern':
                extern = src.code_dom.externc.DOMExternC.parse(context, stream)
                if extern is not None:
                    return extern

            # This could be either a field declaration or a function declaration, so try both

            function_declaration = src.code_dom.functiondeclaration.DOMFunctionDeclaration.parse(context, stream)
            if function_declaration is not None:
                return function_declaration

            field_declaration = src.code_dom.fielddeclaration.DOMFieldDeclaration.parse(context, stream)
            if field_declaration is not None:
                return field_declaration

            # It may be a macro or something else we don't understand, so record it as unparsable and move on
            return src.code_dom.unparsablething.DOMUnparsableThing.parse(context, stream)
        else:
            return None

    # Attach preceding comments
    def attach_preceding_comments(self, comments):
        for comment in comments:
            if comment.parent:
                comment.parent.remove_child(comment)
            self.pre_comments.append(comment)
            comment.parent = self
            comment.is_preceding_comment = True

    # Add an attached comment (if present) to the output line text given, respecting the comment alignment
    def add_attached_comment_to_line(self, line):
        if self.attached_comment is not None:
            padding = self.attached_comment.alignment - len(line)
            padding = max(padding, 1)  # Always have at least one space after the body of the line
            return line + (" " * padding) + self.attached_comment.to_c_string()
        else:
            return line

    # Write any preceding comments
    def write_preceding_comments(self, file, indent=0, context=WriteContext()):
        if context.for_implementation:
            return  # No comments in implementation code
        for comment in self.pre_comments:
            write_c_line(file, indent, comment.to_c_string())

    # Write this element out as C code
    def write_to_c(self, file, indent=0, context=WriteContext()):
        self.write_preceding_comments(file, indent, context)
        write_c_line(file, indent, " // Unsupported element " + str(self))
        for child in self.children:
            child.write_to_c(file, indent + 1, context)
        write_c_line(file, indent, self.add_attached_comment_to_line(" // End of unsupported element " + str(self)))

    # Dump this element for debugging
    def dump(self, indent=0):
        print("".ljust(indent * 4) + str(self))
        for child in self.children:
            child.dump(indent + 1)

    # Gets the fully-qualified name (C++-style) of this element (including namespaces/etc)
    # If include_leading_colons is true then the name will be returned in a genuinely "fully-qualified" fashion -
    # i.e. "::MyClass::Something"
    def get_fully_qualified_name(self, leaf_name="", include_leading_colons=False):
        if self.parent is not None:
            return self.parent.get_fully_qualified_name(leaf_name, include_leading_colons)
        else:
            return ("::" if include_leading_colons else "") + leaf_name

    # Gets the original (i.e. unmodified) fully-qualified name of this element for implementation purposes
    def get_original_fully_qualified_name(self, include_leading_colons=False):
        if self.original_name_override is not None:
            return self.original_name_override
        if self.unmodified_element is not None:
            return self.unmodified_element.get_fully_qualified_name("", include_leading_colons)
        else:
            return self.get_fully_qualified_name("", include_leading_colons)

    # Gets the class/struct that contains this element (if one exists)
    def get_parent_class(self):
        current = self.parent
        while current is not None:
            if isinstance(current, src.code_dom.classstructunion.DOMClassStructUnion):
                return current
            current = current.parent
        return None

    # Add a new child to this element, optionally setting the last element information in the context
    # Removes the child from any previous parent element
    # Note that for elements with multiple child lists this will add to the default one - insert_after_child() is
    # preferably generally for that reason
    def add_child(self, child, context=None):
        if child.parent is not None:
            child.parent.remove_child(child)
        child.parent = self
        self.children.append(child)
        if context is not None:
            context.last_element = child

    # Add multiple children
    def add_children(self, children, context=None):
        for child in children:
            self.add_child(child, context)

    # Remove a child from this element
    def remove_child(self, child):
        if child.parent is not self:
            raise Exception("Attempt to remove child from element other than parent")
        for child_list in self.get_writable_child_lists():
            if child in child_list:
                child_list.remove(child)
                child.parent = None
                return
        # Types are not stored in a list, but are returned in one for traversal purposes. Thus they cannot be
        # removed with remove_child() (because the temporary list returned by get_child_lists() is not returned
        # by get_writable_child_lists()).
        raise Exception("Child not found in any list - this may be because it is attached as a type or similar")

    # Find the element immediately prior to the child given
    def get_prev_child(self, child):
        for child_list in self.get_child_lists():
            for i in range(0, len(child_list)):
                if child_list[i] == child:
                    if i > 0:
                        return child_list[i - 1]
                    else:
                        return None
        raise Exception("Child not found in any list")

    # Find the element immediately after the child given
    def get_next_child(self, child):
        for child_list in self.get_child_lists():
            for i in range(0, len(child_list)):
                if child_list[i] == child:
                    if i < (len(child_list) - 1):
                        return child_list[i + 1]
                    else:
                        return None
        raise Exception("Child not found in any list")

    # Debug function - raises exception if the hierarchy is not valid
    def validate_hierarchy(self):
        for child_list in self.get_child_lists():
            for child in child_list:
                if child.parent is not self:
                    raise Exception("Node " + str(child) + " has parent " + str(child.parent) + " when it should be " +
                                    str(self))
                child.validate_hierarchy()

    # Returns a list of all the lists in this element that contain children
    def get_child_lists(self):
        if self.attached_comment is not None:
            return [self.children, self.pre_comments, [self.attached_comment]]
        else:
            return [self.children, self.pre_comments]

    # Returns a list of all the lists in this element that contain children and can be modified
    # This may be different from get_child_lists in that the former can return temporary lists to enumerate children
    # which are not part of a normal list (e.g. types) and thus cannot be manipulated that way.
    def get_writable_child_lists(self):
        return [self.children, self.pre_comments]

    # Tests if this element is a descendant of (or the same as) the element given
    def is_descendant_of(self, parent):
        if self is parent:
            return True
        if self.parent is None:
            return False
        return self.parent.is_descendant_of(parent)

    # Walk this element and all children, calling a function on them
    def walk(self, func):
        func(self)
        for child_list in self.get_child_lists():
            for child in child_list:
                child.walk(func)

    # Recursively find all the children of this element (and this element itself) that match the type supplied,
    # and return them as a list
    def list_all_children_of_type(self, element_type):
        result = []

        def walker(element):
            if isinstance(element, element_type):
                result.append(element)

        self.walk(walker)

        return result

    # Override for pickling that removes unmodified_element (mainly for cloning, as otherwise we would basically
    # end up cloning the entire unmodified tree every time we cloned anything)
    def __getstate__(self):
        state = self.__dict__.copy()
        if "unmodified_element" in state:
            state["unmodified_element"] = None
        return state

    # Performs a deep clone of this element and all children
    def clone(self):
        # We need to temporarily remove our parent reference to prevent the tree above us getting cloned
        temp_parent = self.parent
        self.parent = None
        clone = copy.deepcopy(self)
        self.parent = temp_parent
        clone.__reconnect_unmodified(self)
        return clone

    # Clone this element but without any children, where "children" means explicit children, such as contained
    # function/fields or similar, but not technically-children like types/arguments/etc. Attached comments are cloned.
    def clone_without_children(self):
        temp_children = self.children
        self.children = []
        clone = self.clone()
        self.children = temp_children
        return clone

    # Reconnect "unmodified_element" on a whole tree of elements
    # (used after cloning, as we don't clone unmodified_element)
    def __reconnect_unmodified(self, original):
        self.unmodified_element = original.unmodified_element

        for child_list, original_child_list in zip(self.get_child_lists(), original.get_child_lists()):
            for child, original_child in zip(child_list, original_child_list):
                child.__reconnect_unmodified(original_child)

    # This creates a clone of this element and all children, stored in the "unmodified_element" field of each
    # corresponding element
    def save_unmodified_clones(self):
        clone = copy.deepcopy(self)
        self.__attach_unmodified_clones(clone)

    # Attach unmodified clones to the tree recursively
    def __attach_unmodified_clones(self, clone):

        # This shouldn't really be necessary, but as a sanity check make sure we're matching (probably) the correct
        # elements to each other
        if str(self) != str(clone):
            raise Exception("Unmodified clone mismatch error")

        self.unmodified_element = clone

        if len(self.get_child_lists()) != len(clone.get_child_lists()):
            raise Exception("Unmodified clone mismatch error")

        for child_list, clone_child_list in zip(self.get_child_lists(), clone.get_child_lists()):
            if len(child_list) != len(clone_child_list):
                raise Exception("Unmodified clone mismatch error")

            for child, clone_child in zip(child_list, clone_child_list):
                child.__attach_unmodified_clones(clone_child)

    # Is this element a preprocessor container (#if or similar)?
    def __is_preprocessor_container(self):
        return False

    # Get a list of the directly contained children of this element - this means all immediate children and
    # all children inside preprocessor #if blocks, but not children of contained structs/namespaces/etc
    # (so in other words, what the C compiler would consider children, after preprocessing has been done)
    def list_directly_contained_children(self):
        result = []
        for child_list in self.get_child_lists():
            for child in child_list:
                if child.__is_preprocessor_container():
                    # Recurse into preprocessor containers
                    for container_child in child.list_directly_contained_children():
                        result.append(container_child)
                else:
                    result.append(child)
        return result

    # Get a list of all directly contained children that match the type supplied
    # (see list_directly_contained_children() for a definition of what "directly contained" means here)
    def list_directly_contained_children_of_type(self, element_type):
        result = []

        for element in self.list_directly_contained_children():
            if isinstance(element, element_type):
                result.append(element)

        return result

    # Replace the direct child element given with one or more new children
    # Removes the child from any previous parent
    def replace_child(self, old_child, new_children):
        old_child.parent = None
        new_children.reverse()  # We're going to insert in backwards order
        for child_list in self.get_child_lists():
            for i in range(0, len(child_list)):
                if child_list[i] == old_child:
                    child_list.remove(old_child)
                    for new_child in new_children:
                        if new_child.parent is not None:
                            new_child.parent.remove_child(new_child)
                        child_list.insert(i, new_child)
                        new_child.parent = self
                    return
        raise Exception("Unable to find child to replace")

    # Insert children before the direct child element given
    # Removes the children from any previous parent
    def insert_before_child(self, existing_child, new_children):
        new_children.reverse()  # We're going to insert in backwards order
        for child_list in self.get_child_lists():
            for i in range(0, len(child_list)):
                if child_list[i] == existing_child:
                    for new_child in new_children:
                        if new_child.parent is not None:
                            new_child.parent.remove_child(new_child)
                        child_list.insert(i, new_child)
                        new_child.parent = self
                    return
        raise Exception("Unable to find child to insert after")

    # Insert children after the direct child element given
    # Removes the children from any previous parent
    def insert_after_child(self, existing_child, new_children):
        new_children.reverse()  # We're going to insert in backwards order
        for child_list in self.get_child_lists():
            for i in range(0, len(child_list)):
                if child_list[i] == existing_child:
                    for new_child in new_children:
                        if new_child.parent is not None:
                            new_child.parent.remove_child(new_child)
                        child_list.insert(i + 1, new_child)
                        new_child.parent = self
                    return
        raise Exception("Unable to find child to insert after")
