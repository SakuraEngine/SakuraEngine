from src import code_dom
from src import utils


# Extract the first template parameter for the template name given from a token list
# Returns the parameter as a string, along with the first and last token indices that
# the parameter occupies (not including any <>s)
# This is kinda shonky and only works for templates with a single parameter
def extract_template_parameter(template_name, tokens):
    for i in range(0, len(tokens) - 3):
        if (tokens[i].value == template_name) and \
                (tokens[i + 1].value == '<'):
            end_index = -1
            for j in range(i + 2, len(tokens)):
                if tokens[j].value == '>':
                    end_index = j
                    break
            if end_index >= 0:
                instantiation_parameter = ""
                for k in range(i + 2, end_index):
                    instantiation_parameter += tokens[k].value
                return instantiation_parameter, i + 2, end_index
    return None, -1, -1


# This modifier finds templates and flattens them, creating concrete classes/functions for each required instantiation
# It currently only supports templates with a single type parameter
# custom_type_fudges can be used to supply strings which will be matched and replaced in modified types within the
# instantiation as a way of working around some issues with the subtleties of template expansion rules (notably
# "const T*" with T as "Blah *" expanding to "Blah* const*" rather than the lexical substitution "const Blah**")
def apply(dom_root, custom_type_fudges={}):
    # Iterate through all templates
    for template in dom_root.list_all_children_of_type(code_dom.DOMTemplate):
        templated_obj = template.get_templated_object()
        template_name = templated_obj.name

        # Figure out the template parameter name
        template_parameter_name = None
        for token in template.template_parameter_tokens:
            if token.value != 'typename':
                if template_parameter_name is not None:
                    raise Exception(
                        "Template " + str(template) +
                        " appears to have more than one parameter (which is not currently supported)")
                template_parameter_name = token.value

        # Instantiation parameters as they exist in the DOM at present
        instantiation_parameters = []
        # Instantiation parameters in their implementation form (if that exists, None if not)
        implementation_instantiation_parameters = []

        # Find all references to this
        for type_element in dom_root.list_all_children_of_type(code_dom.DOMType):
            # Don't look for instantiations inside the template itself (technically this is wrong, but it simplifies
            # things for now as otherwise we'd need to be able to tell the difference between template parameters
            # and concrete types)
            if type_element.is_descendant_of(template):
                continue

            instantiation_parameter, _, _ = extract_template_parameter(template_name, type_element.tokens)

            if instantiation_parameter is not None:
                if instantiation_parameter not in instantiation_parameters:
                    instantiation_parameters.append(instantiation_parameter)

                    # print("Template " + template_name + " referenced in " + str(type_element) + " with parameter " +
                    #      instantiation_parameter)

                    # Figure out what the implementation parameter is and record that
                    implementation_parameter = None
                    if type_element.original_name_override is not None:
                        opening_bracket = type_element.original_name_override.index('<')
                        closing_bracket = type_element.original_name_override.index('>')
                        if (opening_bracket >= 0) and (closing_bracket > opening_bracket):
                            implementation_parameter = \
                                type_element.original_name_override[opening_bracket + 1:closing_bracket]

                    implementation_instantiation_parameters.append(implementation_parameter)

        # Reverse so that when we add these to the DOM (which in turn reverses the order), they end up in the original
        # order they were seen
        instantiation_parameters.reverse()
        implementation_instantiation_parameters.reverse()

        # Duplicate the template for each instantiation

        instantiation_names = []

        for (instantiation_parameter, implementation_instantiation_parameter) in \
                zip(instantiation_parameters, implementation_instantiation_parameters):
            instantiation = templated_obj.clone()
            instantiation.parent = None

            # We need to set up an override so that instead of using the original template typename the
            # implementation uses the name with parameter substitution doe
            if instantiation.original_name_override is None:
                instantiation.original_name_override = instantiation.get_fully_qualified_name()

            # The implementation name should use the implementation version of the instantiation parameter if
            # possible, so we get "ImVector<ImGuiTextFilter::TextRange>" instead of
            # "ImVector<ImGuiTextFilter_TextRange>"
            instantiation.original_name_override += "<" + \
                                                    (implementation_instantiation_parameter or
                                                     instantiation_parameter) + ">"

            # Generate a new name for the instantiation
            instantiation.name += "_" + utils.sanitise_name_for_identifier(instantiation_parameter)
            instantiation_names.append(instantiation.name)

            # Replace all occurrences of the type parameter with the instantiation parameter

            for element in instantiation.list_all_children_of_type(code_dom.DOMType):

                modified_anything = False

                for i in range(0, len(element.tokens)):
                    if element.tokens[i].value == template_parameter_name:
                        element.tokens[i].value = instantiation_parameter
                        modified_anything = True

                if modified_anything:
                    # Do the same for any original name overrides, using the original override version of
                    # the parameter
                    if implementation_instantiation_parameter is not None:
                        if element.original_name_override is None:
                            write_context = code_dom.WriteContext()
                            write_context.for_implementation = True
                            element.original_name_override = element.to_c_string(write_context)
                            element.original_name_override = element.original_name_override \
                                .replace(instantiation_parameter,
                                         implementation_instantiation_parameter)
                        else:
                            # This is kinda dubious because parameter names can be things like T, which will then match
                            # *any* T in the type, but for now I'm banking on that not happening as template types
                            # aren't very complicated and have little aside from the odd "const", * or & on them.
                            element.original_name_override = element.original_name_override \
                                .replace(template_parameter_name,
                                         implementation_instantiation_parameter)

                    # Apply any custom fudges

                    full_type = element.to_c_string()

                    for fudge_key in custom_type_fudges.keys():
                        if fudge_key in full_type:
                            full_type = full_type.replace(fudge_key, custom_type_fudges[fudge_key])

                            # Figure out if any of our source type had reference->pointer conversions done on it
                            num_converted_references = 0
                            for tok in element.tokens:
                                if hasattr(tok, "was_reference") and tok.was_reference:
                                    num_converted_references += 1

                            # Supporting this wouldn't be horrifically difficult, but right now it's hard due to the
                            # way we collapse all the tokens into one here. If this proves necessary then it's probably
                            # a question of either redoing fudges to use token sequences, or adding something to
                            # re-parse the fudged string into tokens and then map the was_reference flag across.
                            if num_converted_references > 1:
                                raise Exception("Fudged type has more than one converted reference - this is not "
                                                "supported")

                            element.tokens = utils.create_tokens_for_type(full_type)

                            if num_converted_references > 0:
                                element.tokens[0].was_reference = True

                            if element.original_name_override is not None:
                                element.original_name_override = element.original_name_override \
                                    .replace(fudge_key, custom_type_fudges[fudge_key])

            # Create a comment to note where this came from
            comment = code_dom.DOMComment()
            comment.comment_text = "// Instantiation of " + template_name + "<" + instantiation_parameter + ">"

            # Optionally insert new struct instances into the DOM at the very end to avoid problems with referencing
            # things that aren't declared yet at the point the template appears
            place_instantiation_at_end = False

            if place_instantiation_at_end:
                # Create a forward-declaration of the template at the point it was originally declared

                declaration_comment = code_dom.DOMComment()
                declaration_comment.comment_text = "// Forward declaration of " + template_name + \
                                                   "<" + instantiation_parameter + ">"

                declaration = instantiation.clone()
                declaration.children.clear()
                declaration.is_forward_declaration = True

                template.parent.insert_after_child(template, [declaration_comment, declaration])

                # Add at end of file
                dom_root.add_children([code_dom.DOMBlankLines(1),
                                       comment,
                                       code_dom.DOMBlankLines(1),
                                       instantiation])
            else:
                # Insert new instance at point of template
                template.parent.insert_after_child(template,
                                                   [code_dom.DOMBlankLines(1),
                                                    comment,
                                                    code_dom.DOMBlankLines(1),
                                                    instantiation])

        # Remove the original template
        template.parent.remove_child(template)

        # Replace any references to the original template types with the new instantiations
        for (instantiation_parameter, instantiation_name) in zip(instantiation_parameters, instantiation_names):
            for type_element in dom_root.list_all_children_of_type(code_dom.DOMType):
                element_instantiation_parameter, first_token, last_token = \
                    extract_template_parameter(template_name, type_element.tokens)
                if element_instantiation_parameter == instantiation_parameter:
                    # Set the original (parameterised) name as the override so it gets used for the
                    # implementation code
                    write_context = code_dom.WriteContext()
                    write_context.use_original_names = True
                    type_element.original_name_override = type_element.to_c_string(write_context)
                    # ...then replace the main name with our instance name

                    # -2 because first_token is the parameter, so we need to step back over the < and the template name
                    first_token_of_reference = first_token - 2

                    type_element.tokens[first_token_of_reference].value = instantiation_name
                    del type_element.tokens[first_token_of_reference + 1:last_token + 1]  # +1 to eat the closing >
