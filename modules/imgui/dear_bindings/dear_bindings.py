# Dear Bindings Version 0.04 WIP
# Generates C-language headers for Dear ImGui
# Developed by Ben Carter (e-mail: ben AT shironekolabs.com, github: @ShironekoBen)

# Example command-line:
#   python dear_bindings.py --output cimgui ../imgui/imgui.h
#   python dear_bindings.py --output cimgui_internal ../imgui/imgui_internal.h   (FIXME: result won't compile yet)

# Example Input:
#   imgui.h     : a C++ header file (aiming to also support imgui_internal.h, implot.h etc.: support is not complete yet).

# Example output:
#   cimgui.h    : a C header file for compilation by a modern C compiler, including full comments from original header file.
#   cimgui.cpp  : a CPP implementation file which can to be linked into a C program.
#   cimgui.json : full metadata to reconstruct bindings for other programming languages, including full comments.

import os
from src import code_dom
from src import c_lexer
import argparse
import sys
import traceback
from src.modifiers import *
from src.generators import *


# Insert a single header template file, complaining if it does not exist
# Replaces any expansions in the expansions dictionary with the given result
def insert_single_template(dest_file, template_file, expansions):
    if not os.path.isfile(template_file):
        print("Template file " + template_file + " could not be found (note that template file names are "
                                                 "expected to match source file names, so if you have "
                                                 "renamed imgui.h you will need to rename the template as "
                                                 "well). The common template file is included regardless of source "
                                                 "file name.")
        sys.exit(2)

    with open(template_file, "r") as src_file:
        for line in src_file.readlines():
            for before, after in expansions.items():
                line = line.replace(before, after)
            dest_file.write(line)


# Insert the contents of the appropriate header template file(s)
def insert_header_templates(dest_file, template_dir, src_file_name, dest_file_ext, expansions):
    # Include the common template file
    insert_single_template(dest_file,
                           os.path.join(template_dir, "common-header-template" + dest_file_ext),
                           expansions)

    # Include the specific template for the file we are generating
    insert_single_template(dest_file,
                           os.path.join(template_dir, src_file_name + "-header-template" + dest_file_ext),
                           expansions)


# Parse the C++ header found in src_file, and write a C header to dest_file_no_ext.h, with binding implementation in
# dest_file_no_ext.cpp. Metadata will be written to dest_file_no_ext.json. implementation_header should point to a file
# containing the initial header block for the implementation (provided in the templates/ directory).
def convert_header(src_file, dest_file_no_ext, template_dir):
    print("Parsing " + src_file)

    with open(src_file, "r") as f:
        file_content = f.read()

    # Tokenize file and then convert into a DOM

    stream = c_lexer.tokenize(file_content)

    if False:  # Debug dump tokens
        while True:
            tok = stream.get_token()
            if not tok:
                break  # No more input
            print(tok)
        return

    context = code_dom.ParseContext()
    dom_root = code_dom.DOMHeaderFileSet()
    dom_root.add_child(code_dom.DOMHeaderFile.parse(context, stream))

    # Assign a filename based on the output file
    _, dom_root.filename = os.path.split(dest_file_no_ext)
    dom_root.filename += ".h"  # Presume the primary output file is the .h

    dom_root.validate_hierarchy()
    #  dom_root.dump()

    print("Storing unmodified DOM")

    dom_root.save_unmodified_clones()

    print("Applying modifiers")

    # Apply modifiers

    # Add headers we need and remove those we don't
    mod_add_includes.apply(dom_root, ["<stdbool.h>"])  # We need stdbool.h to get bool defined
    mod_remove_includes.apply(dom_root, ["<float.h>",
                                         "<stdarg.h>",
                                         "<stddef.h>",
                                         "<string.h>"])

    mod_attach_preceding_comments.apply(dom_root)
    mod_remove_function_bodies.apply(dom_root)
    mod_assign_anonymous_type_names.apply(dom_root)
    # Remove ImGuiOnceUponAFrame for now as it needs custom fiddling to make it usable from C
    # Remove ImNewDummy/ImNewWrapper as it's a helper for C++ new (and C dislikes empty structs)
    mod_remove_structs.apply(dom_root, ["ImGuiOnceUponAFrame",
                                        "ImNewDummy",  # ImGui <1.82
                                        "ImNewWrapper",  # ImGui >=1.82
                                        # Templated stuff in imgui_internal.h
                                        "ImBitArray",
                                        "ImBitVector",
                                        "ImSpan",
                                        "ImSpanAllocator",
                                        "ImPool",
                                        "ImChunkStream",
                                        "ImGuiTextIndex"])
    # Remove all functions from ImVector, as they're not really useful
    mod_remove_all_functions_from_classes.apply(dom_root, ["ImVector"])
    # Remove Value() functions which are dumb helpers over Text(), would need custom names otherwise
    mod_remove_functions.apply(dom_root, ["ImGui::Value"])
    # Remove ImQsort() functions as modifiers on function pointers seem to emit a "anachronism used: modifiers on data are ignored" warning.
    mod_remove_functions.apply(dom_root, ["ImQsort"])
    # FIXME: Remove incorrectly parsed constructor due to "explicit" keyword.
    mod_remove_functions.apply(dom_root, ["ImVec2ih::ImVec2ih"])
    # Remove some templated functions from imgui_internal.h that we don't want and cause trouble
    mod_remove_functions.apply(dom_root, ["ImGui::ScaleRatioFromValueT",
                                          "ImGui::ScaleValueFromRatioT",
                                          "ImGui::DragBehaviorT",
                                          "ImGui::SliderBehaviorT",
                                          "ImGui::RoundScalarWithFormatT",
                                          "ImGui::CheckboxFlagsT"])
    mod_add_prefix_to_loose_functions.apply(dom_root, "c")

    # Add helper functions to create/destroy ImVectors
    # Implementation code for these can be found in templates/imgui-header.cpp
    mod_add_manual_helper_functions.apply(dom_root,
                                          [
                                              "void ImVector_Construct(void* vector); // Construct a "
                                              "zero-size ImVector<> (of any type). This is primarily "
                                              "useful when calling "
                                              "ImFontGlyphRangesBuilder_BuildRanges()",

                                              "void ImVector_Destruct(void* vector); // Destruct an "
                                              "ImVector<> (of any type). Important: Frees the vector "
                                              "memory but does not call destructors on contained objects "
                                              "(if they have them)",
                                          ])
    # ImStr conversion helper, only enabled if IMGUI_HAS_IMSTR is on
    mod_add_manual_helper_functions.apply(dom_root,
                                          [
                                              "ImStr ImStr_FromCharStr(const char* b); // Build an ImStr "
                                              "from a regular const char* (no data is copied, so you need to make "
                                              "sure the original char* isn't altered as long as you are using the "
                                              "ImStr)."
                                          ],
                                          # This weirdness is because we want this to compile cleanly even if
                                          # IMGUI_HAS_IMSTR wasn't defined
                                          ["defined(IMGUI_HAS_IMSTR)", "IMGUI_HAS_IMSTR"])

    # Add a note to ImFontGlyphRangesBuilder_BuildRanges() pointing people at the helpers
    mod_add_function_comment.apply(dom_root,
                                   "ImFontGlyphRangesBuilder::BuildRanges",
                                   "(ImVector_Construct()/ImVector_Destruct() can be used to safely "
                                   "construct out_ranges)")

    mod_remove_operators.apply(dom_root)
    mod_remove_heap_constructors_and_destructors.apply(dom_root)
    mod_convert_references_to_pointers.apply(dom_root)
    # Assume IM_VEC2_CLASS_EXTRA and IM_VEC4_CLASS_EXTRA are never defined as they are likely to just cause problems
    # if anyone tries to use it
    mod_flatten_conditionals.apply(dom_root, "IM_VEC2_CLASS_EXTRA", False)
    mod_flatten_conditionals.apply(dom_root, "IM_VEC4_CLASS_EXTRA", False)
    mod_flatten_namespaces.apply(dom_root, {'ImGui': 'ImGui_'})
    mod_flatten_nested_classes.apply(dom_root)
    # The custom type fudge here is a workaround for how template parameters are expanded
    mod_flatten_templates.apply(dom_root, custom_type_fudges={'const ImFont**': 'ImFont* const*'})
    # We treat ImVec2, ImVec4 and ImColor as by-value types
    mod_mark_by_value_structs.apply(dom_root, by_value_structs=['ImVec2', 'ImVec4', 'ImColor', 'ImStr'])
    mod_mark_internal_members.apply(dom_root)
    mod_flatten_class_functions.apply(dom_root)
    mod_remove_nested_typedefs.apply(dom_root)
    mod_remove_static_fields.apply(dom_root)
    mod_remove_constexpr.apply(dom_root)
    mod_generate_imstr_helpers.apply(dom_root)
    mod_remove_enum_forward_declarations.apply(dom_root)
    mod_disambiguate_functions.apply(dom_root,
                                     name_suffix_remaps={
                                         # Some more user-friendly suffixes for certain types
                                         'const char*': 'Str',
                                         'char*': 'Str',
                                         'unsigned int': 'Uint',
                                         'unsigned int*': 'UintPtr',
                                         'ImGuiID': 'ID',
                                         'const void*': 'Ptr',
                                         'void*': 'Ptr'},
                                     # Functions that look like they have name clashes but actually don't
                                     # thanks to preprocessor conditionals
                                     functions_to_ignore=[
                                         "cImFileOpen",
                                         "cImFileClose",
                                         "cImFileGetSize",
                                         "cImFileRead",
                                         "cImFileWrite"],
                                     functions_to_rename_everything=[
                                         "ImGui_CheckboxFlags"  # This makes more sense as IntPtr/UIntPtr variants
                                     ],
                                     type_priorities={
                                     })
    mod_generate_default_argument_functions.apply(dom_root,
                                                  # We ignore functions that don't get called often because in those
                                                  # cases the default helper doesn't add much value but does clutter
                                                  # up the header file
                                                  functions_to_ignore=[
                                                      # Main
                                                      'ImGui_CreateContext',
                                                      'ImGui_DestroyContext',
                                                      # Demo, Debug, Information
                                                      'ImGui_ShowDemoWindow',
                                                      'ImGui_ShowMetricsWindow',
                                                      'ImGui_ShowDebugLogWindow',
                                                      'ImGui_ShowStackToolWindow',
                                                      'ImGui_ShowAboutWindow',
                                                      'ImGui_ShowStyleEditor',
                                                      # Styles
                                                      'ImGui_StyleColorsDark',
                                                      'ImGui_StyleColorsLight',
                                                      'ImGui_StyleColorsClassic',
                                                      # Windows
                                                      'ImGui_Begin',
                                                      'ImGui_BeginChild',
                                                      'ImGui_BeginChildID',
                                                      'ImGui_SetNextWindowSizeConstraints',
                                                      # Scrolling
                                                      'ImGui_SetScrollHereX',
                                                      'ImGui_SetScrollHereY',
                                                      'ImGui_SetScrollFromPosX',
                                                      'ImGui_SetScrollFromPosY',
                                                      # Parameters stacks
                                                      'ImGui_PushTextWrapPos',
                                                      # Widgets
                                                      'ImGui_ProgressBar',
                                                      'ImGui_ColorPicker4',
                                                      'ImGui_TreePushPtr', # Ensure why core lib has this default to NULL?
                                                      'ImGui_BeginListBox',
                                                      'ImGui_ListBox',
                                                      'ImGui_MenuItemBoolPtr',
                                                      'ImGui_BeginPopupModal',
                                                      'ImGui_OpenPopupOnItemClick',
                                                      'ImGui_TableGetColumnName',
                                                      'ImGui_TableGetColumnFlags',
                                                      'ImGui_TableSetBgColor',
                                                      'ImGui_GetColumnWidth',
                                                      'ImGui_GetColumnOffset',
                                                      'ImGui_BeginTabItem',
                                                      # Misc
                                                      'ImGui_LogToTTY',
                                                      'ImGui_LogToFile',
                                                      'ImGui_LogToClipboard',
                                                      'ImGui_BeginDisabled',
                                                      # Inputs
                                                      'ImGui_IsMousePosValid',
                                                      'ImGui_IsMouseDragging',
                                                      'ImGui_GetMouseDragDelta',
                                                      'ImGui_CaptureKeyboardFromApp',
                                                      'ImGui_CaptureMouseFromApp',
                                                      # Settings
                                                      'ImGui_LoadIniSettingsFromDisk',
                                                      'ImGui_LoadIniSettingsFromMemory',
                                                      'ImGui_SaveIniSettingsToMemory',
                                                      'ImGui_SaveIniSettingsToMemory',
                                                      # Memory Allcators
                                                      'ImGui_SetAllocatorFunctions',
                                                      # Other types
                                                      'ImGuiIO_SetKeyEventNativeDataEx',
                                                      'ImGuiTextFilter_Draw',
                                                      'ImGuiTextFilter_PassFilter',
                                                      'ImGuiTextBuffer_append',
                                                      'ImGuiInputTextCallbackData_InsertChars',
                                                      'ImColor_SetHSV',
                                                      'ImColor_HSV',
                                                      'ImGuiListClipper_Begin',
                                                      # ImDrawList
                                                      # - all 'int num_segments = 0' made explicit
                                                      'ImDrawList_AddCircleFilled',
                                                      'ImDrawList_AddBezierCubic',
                                                      'ImDrawList_AddBezierQuadratic',
                                                      'ImDrawList_PathStroke',
                                                      'ImDrawList_PathArcTo',
                                                      'ImDrawList_PathBezierCubicCurveTo',
                                                      'ImDrawList_PathBezierQuadraticCurveTo',
                                                      'ImDrawList_PathRect',
                                                      'ImDrawList_AddBezierCurve',
                                                      'ImDrawList_PathBezierCurveTo',
                                                      'ImDrawList_PushClipRect',
                                                      # ImFont, ImFontGlyphRangesBuilder
                                                      'ImFontGlyphRangesBuilder_AddText',
                                                      'ImFont_AddRemapChar',
                                                      'ImFont_RenderText',
                                                      # Obsolete functions
                                                      'ImGui_ImageButtonImTextureID',
                                                      'ImGui_ListBoxHeaderInt',
                                                      'ImGui_ListBoxHeader',
                                                      'ImGui_OpenPopupContextItem',
                                                  ],
                                                  function_prefixes_to_ignore=[
                                                      'ImGuiStorage_',
                                                      'ImFontAtlas_'
                                                  ],
                                                  trivial_argument_types=[
                                                      'ImGuiCond'
                                                  ],
                                                  trivial_argument_names=[
                                                      'flags',
                                                      'popup_flags'
                                                  ])

    # Do some special-case renaming of functions
    mod_rename_functions.apply(dom_root, {
        # We want the ImGuiCol version of GetColorU32 to be the primary one, but we can't use type_priorities on
        # mod_disambiguate_functions to achieve that because it also has more arguments and thus naturally gets passed
        # over. Rather than introducing yet another layer of knobs to try and control _that_, we just do some
        # after-the-fact renaming here.
        'ImGui_GetColorU32': 'ImGui_GetColorU32ImVec4',
        'ImGui_GetColorU32ImGuiCol': 'ImGui_GetColorU32',
        'ImGui_GetColorU32ImGuiColEx': 'ImGui_GetColorU32Ex',
        # ImGui_IsRectVisible is kinda inobvious as it stands, since the two overrides take the exact same type but
        # interpret it differently. Hence do some renaming to make it clearer.
        'ImGui_IsRectVisible': 'ImGui_IsRectVisibleBySize',
        'ImGui_IsRectVisibleImVec2': 'ImGui_IsRectVisible'
    })

    # Make all functions use CIMGUI_API
    mod_make_all_functions_use_imgui_api.apply(dom_root)
    mod_rename_defines.apply(dom_root, {'IMGUI_API': 'CIMGUI_API'})

    mod_forward_declare_structs.apply(dom_root)
    mod_wrap_with_extern_c.apply(dom_root)
    # For now we leave #pragma once intact on the assumption that modern compilers all support it, but if necessary
    # it can be replaced with a traditional #include guard by uncommenting the line below. If you find yourself needing
    # this functionality in a significant way please let me know!
    # mod_remove_pragma_once.apply(dom_root)
    mod_remove_empty_conditionals.apply(dom_root)
    mod_merge_blank_lines.apply(dom_root)
    mod_remove_blank_lines.apply(dom_root)
    mod_align_enum_values.apply(dom_root)
    mod_align_function_names.apply(dom_root)
    mod_align_structure_field_names.apply(dom_root)
    mod_align_comments.apply(dom_root)

    # Exclude some defines that aren't really useful from the metadata
    mod_exclude_defines_from_metadata.apply(dom_root, [
        "IMGUI_IMPL_API",
        "IM_COL32_WHITE",
        "IM_COL32_BLACK",
        "IM_COL32_BLACK_TRANS",
        "ImDrawCallback_ResetRenderState",
    ])

    dom_root.validate_hierarchy()

    # dom_root.dump()

    # Cases where the varargs list version of a function does not simply have a V added to the name and needs a
    # custom suffix instead
    custom_varargs_list_suffixes = {'appendf': 'v'}

    # Get just the name portion of the source file, to use as the template name
    src_file_name_only = os.path.splitext(os.path.basename(src_file))[0]

    print("Writing output to " + dest_file_no_ext + "[.h/.cpp/.json]")

    dest_file_name_only = os.path.basename(dest_file_no_ext)

    # If our output name ends with _internal, then generate a version of it without that on the assumption that
    # this is probably imgui_internal.h and thus we need to know what imgui.h is (likely) called as well.
    if dest_file_name_only.endswith('_internal'):
        dest_file_name_only_no_internal = dest_file_name_only[:-9]
    else:
        dest_file_name_only_no_internal = dest_file_name_only

    # Expansions to be used when processing templates, to insert variables as required
    expansions = {"%OUTPUT_HEADER_NAME%": dest_file_name_only + ".h",
                  "%OUTPUT_HEADER_NAME_NO_INTERNAL%": dest_file_name_only_no_internal + ".h"}

    with open(dest_file_no_ext + ".h", "w") as file:
        insert_header_templates(file, template_dir, src_file_name_only, ".h", expansions)

        write_context = code_dom.WriteContext()
        write_context.for_c = True
        dom_root.write_to_c(file, context=write_context)

    # Generate implementations
    with open(dest_file_no_ext + ".cpp", "w") as file:
        insert_header_templates(file, template_dir, src_file_name_only, ".cpp", expansions)

        gen_struct_converters.generate(dom_root, file, indent=0)

        gen_function_stubs.generate(dom_root, file, indent=0,
                                    custom_varargs_list_suffixes=custom_varargs_list_suffixes)

    # Generate metadata
    with open(dest_file_no_ext + ".json", "w") as file:
        gen_metadata.generate(dom_root, file)


if __name__ == '__main__':
    # Parse the C++ header found in src_file, and write a C header to dest_file_no_ext.h, with binding implementation in
    # dest_file_no_ext.cpp. Metadata will be written to dest_file_no_ext.json. implementation_header should point to a
    # file containing the initial header block for the implementation (provided in the templates/ directory).

    print("Dear Bindings: parse Dear ImGui headers, convert to C and output metadata.")

    parser = argparse.ArgumentParser(
                        add_help=True,
                        epilog='Result code 0 is returned on success, 1 on conversion failure and 2 on '
                               'parameter errors')
    parser.add_argument('src',
                        help='Path to source header file to process (generally imgui.h)')
    parser.add_argument('-o', '--output',
                        required=True,
                        help='Path to output files (generally cimgui). This should have no extension, '
                             'as <output>.h, <output>.cpp and <output>.json will be written.')
    parser.add_argument('-t', '--templatedir',
                        default="./src/templates",
                        help='Path to the implementation template directory (default: ./src/templates)')

    if len(sys.argv)==1:
        parser.print_help(sys.stderr)
        sys.exit(0)

    args = parser.parse_args()

    # Perform conversion
    try:
        convert_header(args.src, args.output, args.templatedir)
    except:  # noqa - suppress warning about broad exception clause as it's intentionally broad
        print("Exception during conversion:")
        traceback.print_exc()
        sys.exit(1)

    print("Done")
    sys.exit(0)
