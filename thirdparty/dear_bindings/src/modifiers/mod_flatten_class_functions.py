from src import code_dom
from src import utils


# This modifier converts any classes to structs, and moves member functions inside classes/structs outside
def apply(dom_root):
    # Iterate through all structs/classes/unions
    for struct in dom_root.list_all_children_of_type(code_dom.DOMClassStructUnion):
        # Convert classes to structs
        if struct.structure_type == "CLASS":
            struct.structure_type = "STRUCT"

        current_add_point = struct

        is_by_value = struct.is_by_value

        # Find any child functions
        # Note that this doesn't handle functions in nested classes correctly
        # (but that isn't an issue because we flatten them beforehand)
        for function in struct.list_all_children_of_type(code_dom.DOMFunctionDeclaration):

            # Special handling for constructors/destructors
            if function.is_constructor:
                # Constructors get modified to return a pointer to the newly-created object
                function.return_type = code_dom.DOMType()
                if is_by_value:
                    # By-value types have constructors that return by-value, unsurprisingly
                    function.return_type.tokens = [utils.create_token(struct.name)]
                else:
                    function.return_type.tokens = [utils.create_token(struct.name),
                                                   utils.create_token("*")]
                function.return_type.parent = function
                # Make a note for the code generator that this is a by-value constructor
                function.is_by_value_constructor = is_by_value
            elif function.is_destructor:
                if is_by_value:
                    # We don't support this for fairly obvious reasons
                    raise Exception("By-value type " + struct.name + " has a destructor")
                # Remove ~ and add suffix to name
                function.name = function.name[1:] + "_destroy"
                # Destructors get modified to return void
                function.return_type = code_dom.DOMType()
                function.return_type.tokens = [utils.create_token("void")]
                function.return_type.parent = function

            # Prefix structure name onto the function
            function.name = struct.name + "_" + function.name

            # Note the class it came from originally
            function.original_class = struct

            if not function.is_constructor:
                # Add a self argument as the first argument of the function
                self_arg = code_dom.DOMFunctionArgument()
                self_arg.arg_type = code_dom.DOMType()
                self_arg.arg_type.parent = self_arg
                self_arg.arg_type.tokens = [utils.create_token(struct.name), utils.create_token("*")]

                # Make self const if the original function was const
                if function.is_const:
                    self_arg.arg_type.tokens.insert(0, utils.create_token("const"))

                self_arg.name = "self"
                self_arg.parent = function
                function.arguments.insert(0, self_arg)

            # Remove const-ness as that has no meaning when the function is moved outside
            # (and we've applied const to the self parameter, which achieves the same effect in C-land)
            function.is_const = False

            # Move the function out into the scope the structure is in, adding/removing preprocessor conditionals
            # as required to make sure the function declaration is subject to the same conditions after the move

            # See if we need to change any conditionals
            add_point_conditionals = utils.get_preprocessor_conditionals(current_add_point)
            wanted_conditionals = utils.get_preprocessor_conditionals(function)

            while (len(add_point_conditionals) > len(wanted_conditionals)) or \
                    ((len(add_point_conditionals) > 0) and
                     (not add_point_conditionals[len(add_point_conditionals) - 1]
                        .condition_matches(wanted_conditionals[len(add_point_conditionals) - 1]))):
                # We need to remove a conditional
                conditional = add_point_conditionals.pop(len(add_point_conditionals) - 1)
                if not current_add_point.parent.condition_matches(conditional):
                    # In broad theoretical terms this *should* be impossible, but there may be some corner-case where
                    # a pre-existing conditional in the DOM somehow gets used in a weird way or another element happens
                    # to get inserted between things here
                    raise Exception("Needed to remove conditional " + str(conditional) + " but it wasn't the parent")
                current_add_point = current_add_point.parent

            add_inside_conditional = None  # If we need to add the function into a conditional, this will be it

            # Add any new conditionals that are needed
            while len(add_point_conditionals) < len(wanted_conditionals):
                conditional = wanted_conditionals[len(add_point_conditionals)]
                new_conditional = conditional.clone_without_children()
                new_conditional.parent = None

                if add_inside_conditional is not None:
                    add_inside_conditional.add_child(new_conditional)
                else:
                    current_add_point.parent.insert_after_child(current_add_point, [new_conditional])

                add_point_conditionals.append(new_conditional)
                add_inside_conditional = new_conditional

            if add_inside_conditional is not None:
                add_inside_conditional.add_child(function)
            else:
                current_add_point.parent.insert_after_child(current_add_point, [function])
            current_add_point = function  # Add next function after this one
