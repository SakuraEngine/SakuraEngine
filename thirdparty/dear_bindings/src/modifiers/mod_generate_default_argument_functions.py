from src import code_dom


# This modifier generates variants of any function which has default arguments that takes only the required ones
# (for ease of programming, since C doesn't support default arguments)
# functions_to_ignore contains a list of functions that should not have variants generated
# function_prefixes_to_ignore contains a list of function prefixes that should not have variants generated
# trivial_argument_types is a list of argument types that will not get defaulted where possible if the default is 0/null
# trivial_argument_names is a list of argument names that will not get defaulted where possible if the default is 0/null
def apply(dom_root,
          functions_to_ignore=[],
          function_prefixes_to_ignore=[],
          trivial_argument_types=[],
          trivial_argument_names=[]):

    # Should functions with default arguments appear after the no-defaults ("ex") version of the function or before it?
    add_defaulted_functions_before_ex = True

    for function in dom_root.list_all_children_of_type(code_dom.DOMFunctionDeclaration):
        # Ignore based on exact name match
        if function.name in functions_to_ignore:
            continue

        # Ignore based on prefix match
        ignored_prefix = False
        for prefix in function_prefixes_to_ignore:
            if function.name.startswith(prefix):
                ignored_prefix = True
                continue
        if ignored_prefix:
            continue

        num_args = len(function.arguments)
        num_args_with_defaults = 0
        has_non_defaulted_imstr_args = False

        for arg in function.arguments:
            if arg.default_value_tokens is not None:
                num_args_with_defaults += 1
            else:
                if (not arg.is_varargs) and (arg.arg_type.to_c_string() == 'ImStr'):
                    has_non_defaulted_imstr_args = True

        if (num_args == 0) or (num_args_with_defaults == 0):
            continue  # Nothing useful to do

        # Don't generate default argument helpers for ImStr functions that have had ImStr conversion helpers generated,
        # unless they also contain a non-defaulted ImStr argument (because if all the string arguments are defaulted,
        # the original ImStr function and the char* conversion helper will end up being identical after default
        # arguments have been removed, rendering one of them pointless. In that case we remove the non-conversion helper
        # variant because we generally expect C API users to be using the conversion helpers, and they are also the
        # primary target audience for the default argument helpers too).
        if function.has_imstr_helper and not has_non_defaulted_imstr_args:
            continue

        # Check how many of the first default arguments are trivial, and leave those without a default even in the
        # with-defaults function (rationale: there are many cases where flags/etc can be trivially specified without
        # cluttering the API, and conversely having to use the Ex function with all arguments is overkill if only
        # they need to be changed).

        for i in range(num_args - num_args_with_defaults, num_args):
            # Only consider leaving arguments if they have a zero or null default
            if (function.arguments[i].get_default_value() == '0') or \
               (function.arguments[i].get_default_value() == 'NULL'):
                if (function.arguments[i].name in trivial_argument_names) or \
                   (function.arguments[i].arg_type.get_fully_qualified_name() in trivial_argument_types):
                    # Reduce the number of arguments we will apply defaults to
                    num_args_with_defaults -= 1
                else:
                    # Once a single argument we do intend to default has been found, we need to stop
                    break
            else:
                break

        if num_args_with_defaults == 0:
            continue  # Nothing useful left to do

        # Clone the function and set the defaulted arguments

        new_function = function.clone()

        function.name += "Ex"  # Alter the name of the non-defaulted function

        for i in range(num_args - num_args_with_defaults, num_args):
            new_function.arguments[i].is_implicit_default = True
        new_function.is_default_argument_helper = True

        # Generate a comment noting the defaulted arguments

        comment_text = "// Implied "

        first = True
        for i in range(num_args - num_args_with_defaults, num_args):
            if not first:
                comment_text += ", "
            arg = function.arguments[i]
            if arg.name:
                comment_text += arg.name + " = "
            comment_text += code_dom.common.collapse_tokens_to_string(arg.default_value_tokens)
            first = False

        # Attach that comment and remove any others

        new_function.pre_comments = []

        comment = code_dom.DOMComment.from_string(comment_text)
        comment.is_attached_comment = True
        comment.parent = new_function

        new_function.attached_comment = comment

        # Insert new function

        if add_defaulted_functions_before_ex:
            # When inserting before the Ex function, we need to move any pre-comments up onto ourselves
            comments_to_move = function.pre_comments.copy()  # Copy to avoid altering the list as we iterate
            new_function.attach_preceding_comments(comments_to_move)

            function.parent.insert_before_child(function, [new_function])
        else:
            function.parent.insert_after_child(function, [new_function])
