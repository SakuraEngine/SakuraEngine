using SB;
using SB.Core;

[TargetScript(TargetCategory.Package)]
public static class DaScriptCore
{
    static DaScriptCore()
    {
        BuildSystem
            .Package("DaScriptCore")
            .AddTarget("UriParser", (Target Target, PackageConfig Config) =>
            {
                Target
                    .TargetType(TargetType.Static)
                    .AddCFiles(
                        "daScript/3rdparty/uriparser/src/UriCommon.c",
                        "daScript/3rdparty/uriparser/src/UriCompare.c",
                        "daScript/3rdparty/uriparser/src/UriEscape.c",
                        "daScript/3rdparty/uriparser/src/UriFile.c",
                        "daScript/3rdparty/uriparser/src/UriIp4.c",
                        "daScript/3rdparty/uriparser/src/UriIp4Base.c",
                        "daScript/3rdparty/uriparser/src/UriNormalize.c",
                        "daScript/3rdparty/uriparser/src/UriNormalizeBase.c",
                        "daScript/3rdparty/uriparser/src/UriParse.c",
                        "daScript/3rdparty/uriparser/src/UriParseBase.c",
                        "daScript/3rdparty/uriparser/src/UriQuery.c",
                        "daScript/3rdparty/uriparser/src/UriRecompose.c",
                        "daScript/3rdparty/uriparser/src/UriResolve.c",
                        "daScript/3rdparty/uriparser/src/UriShorten.c",
                        "daScript/3rdparty/uriparser/src/UriMemory.c"
                    )
                    .Defines(Visibility.Public, "URIPARSER_BUILD_CHAR", "URI_STATIC_BUILD")
                    .IncludeDirs(Visibility.Public, "daScript/3rdparty/uriparser/include");
            })
            .AddTarget("DaScriptCore", (Target Target, PackageConfig Config) =>
            {
                if (BuildSystem.TargetOS == OSPlatform.Windows)
                {
                    Target
                        .CppFlags(Visibility.Private, "/bigobj", "/Zc:threadSafeInit")
                        .Defines(Visibility.Public, "NOMINMAX", "_CRT_SECURE_NO_WARNINGS");
                }

                Target
                    .TargetType(TargetType.Static)
                    .Exception(true)
                    .CppVersion("17")
                    .Require("DaScriptCore", Config)
                    .Depend(Visibility.Public, "DaScriptCore@UriParser")
                    .IncludeDirs(Visibility.Public, "daScript/include")
                    // parser
                    .AddCppFiles(
                        // "daScript/src/parser/ds_parser.hpp",
                        "daScript/src/parser/ds_parser.cpp",
                        // "daScript/src/parser/ds_parser.output",
                        // "daScript/src/parser/lex.yy.h",
                        "daScript/src/parser/ds_lexer.cpp"
                    )
                    .AddCppFiles(
                        // "daScript/src/parser/ds_parser.ypp",
                        // "daScript/src/parser/ds_lexer.lpp",
                        // "daScript/src/parser/parser_state.h",
                        // "daScript/src/parser/parser_impl.h"
                        "daScript/src/parser/parser_impl.cpp"
                    )
                    // vecmath
                    // .AddCppFiles(
                        // "daScript/include/vecmath/dag_vecMath.h",
                        // "daScript/include/vecmath/dag_vecMathDecl.h",
                        // "daScript/include/vecmath/dag_vecMath_common.h",
                        // "daScript/include/vecmath/dag_vecMath_const.h",
                        // "daScript/include/vecmath/dag_vecMath_neon.h",
                        // "daScript/include/vecmath/dag_vecMath_pc_sse.h"
                    // )
                    // ast
                    .AddCppFiles(
                        "daScript/src/ast/ast.cpp",
                        "daScript/src/ast/ast_tls.cpp",
                        "daScript/src/ast/ast_visitor.cpp",
                        "daScript/src/ast/ast_generate.cpp",
                        "daScript/src/ast/ast_simulate.cpp",
                        "daScript/src/ast/ast_typedecl.cpp",
                        "daScript/src/ast/ast_match.cpp",
                        "daScript/src/ast/ast_module.cpp",
                        "daScript/src/ast/ast_print.cpp",
                        "daScript/src/ast/ast_aot_cpp.cpp",
                        "daScript/src/ast/ast_infer_type.cpp",
                        "daScript/src/ast/ast_lint.cpp",
                        "daScript/src/ast/ast_allocate_stack.cpp",
                        "daScript/src/ast/ast_derive_alias.cpp",
                        "daScript/src/ast/ast_const_folding.cpp",
                        "daScript/src/ast/ast_block_folding.cpp",
                        "daScript/src/ast/ast_unused.cpp",
                        "daScript/src/ast/ast_annotations.cpp",
                        "daScript/src/ast/ast_export.cpp",
                        "daScript/src/ast/ast_parse.cpp",
                        "daScript/src/ast/ast_debug_info_helper.cpp",
                        "daScript/src/ast/ast_handle.cpp"
                    )
                    // builtin
                    .AddCppFiles(
                        "daScript/src/builtin/module_builtin.cpp",
                        "daScript/src/builtin/module_builtin_misc_types.cpp",
                        "daScript/src/builtin/module_builtin_runtime.cpp",
                        "daScript/src/builtin/module_builtin_runtime_sort.cpp",
                        "daScript/src/builtin/module_builtin_runtime_lockcheck.cpp",
                        "daScript/src/builtin/module_builtin_vector.cpp",
                        "daScript/src/builtin/module_builtin_vector_ctor.cpp",
                        "daScript/src/builtin/module_builtin_array.cpp",
                        "daScript/src/builtin/module_builtin_das.cpp",
                        "daScript/src/builtin/module_builtin_math.cpp",
                        "daScript/src/builtin/module_builtin_raster.cpp",
                        "daScript/src/builtin/module_builtin_string.cpp",
                        "daScript/src/builtin/module_builtin_rtti.cpp",
                        "daScript/src/builtin/module_builtin_ast.cpp",
                        "daScript/src/builtin/module_builtin_ast_flags.cpp",
                        "daScript/src/builtin/module_builtin_ast_annotations.cpp",
                        "daScript/src/builtin/module_builtin_ast_annotations_1.cpp",
                        "daScript/src/builtin/module_builtin_ast_annotations_2.cpp",
                        "daScript/src/builtin/module_builtin_ast_annotations_3.cpp",
                        "daScript/src/builtin/module_builtin_ast_adapters.cpp",
                        "daScript/src/builtin/module_builtin_uriparser.cpp",
                        "daScript/src/builtin/module_jit.cpp",
                        "daScript/src/builtin/module_builtin_fio.cpp",
                        "daScript/src/builtin/module_builtin_dasbind.cpp",
                        "daScript/src/builtin/module_builtin_network.cpp",
                        "daScript/src/builtin/module_builtin_debugger.cpp",
                        "daScript/src/builtin/module_builtin_jobque.cpp",
                        "daScript/src/builtin/module_file_access.cpp"
                        // "daScript/src/builtin/builtin.das.inc",
                        // "daScript/src/builtin/builtin.das",
                        // "daScript/src/builtin/fio.das.inc",
                        // "daScript/src/builtin/fio.das",
                        // "daScript/src/builtin/rtti.das.inc",
                        // "daScript/src/builtin/rtti.das",
                        // "daScript/src/builtin/ast.das.inc",
                        // "daScript/src/builtin/ast.das",
                        // "daScript/src/builtin/network.das.inc",
                        // "daScript/src/builtin/network.das",
                        // "daScript/src/builtin/debugger.das.inc",
                        // "daScript/src/builtin/debugger.das"
                    )
                    // misc
                    .AddCppFiles(
                        "daScript/src/misc/sysos.cpp",
                        "daScript/src/misc/string_writer.cpp",
                        "daScript/src/misc/memory_model.cpp",
                        "daScript/src/misc/job_que.cpp",
                        "daScript/src/misc/free_list.cpp",
                        "daScript/src/misc/daScriptC.cpp",
                        "daScript/src/misc/uric.cpp"
                    )
                    // simulation fusion
                    .AddCppFiles(
                        "daScript/src/simulate/simulate_fusion.cpp",
                        "daScript/src/simulate/simulate_fusion_op1.cpp",
                        "daScript/src/simulate/simulate_fusion_op1_return.cpp",
                        "daScript/src/simulate/simulate_fusion_ptrfdr.cpp",
                        "daScript/src/simulate/simulate_fusion_op2.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_set.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_bool.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_bin.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_vec.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_set_vec.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_bool_vec.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_bin_vec.cpp",
                        "daScript/src/simulate/simulate_fusion_op2_scalar_vec.cpp",
                        "daScript/src/simulate/simulate_fusion_at.cpp",
                        "daScript/src/simulate/simulate_fusion_at_array.cpp",
                        "daScript/src/simulate/simulate_fusion_tableindex.cpp",
                        "daScript/src/simulate/simulate_fusion_misc_copy.cpp",
                        "daScript/src/simulate/simulate_fusion_call1.cpp",
                        "daScript/src/simulate/simulate_fusion_call2.cpp",
                        "daScript/src/simulate/simulate_fusion_if.cpp"
                    )
                    // simulate src
                    .AddCppFiles(
                        "daScript/src/hal/performance_time.cpp",
                        "daScript/src/hal/debug_break.cpp",
                        "daScript/src/hal/project_specific.cpp",
                        "daScript/src/hal/project_specific_file_info.cpp",
                        "daScript/src/misc/network.cpp",
                        "daScript/src/simulate/hash.cpp",
                        "daScript/src/simulate/debug_info.cpp",
                        "daScript/src/simulate/runtime_string.cpp",
                        "daScript/src/simulate/runtime_array.cpp",
                        "daScript/src/simulate/runtime_table.cpp",
                        "daScript/src/simulate/runtime_profile.cpp",
                        "daScript/src/simulate/simulate.cpp",
                        "daScript/src/simulate/simulate_exceptions.cpp",
                        "daScript/src/simulate/simulate_gc.cpp",
                        "daScript/src/simulate/simulate_tracking.cpp",
                        "daScript/src/simulate/simulate_visit.cpp",
                        "daScript/src/simulate/simulate_print.cpp",
                        "daScript/src/simulate/simulate_fn_hash.cpp",
                        "daScript/src/simulate/simulate_instrument.cpp",
                        "daScript/src/simulate/heap.cpp",
                        "daScript/src/simulate/data_walker.cpp",
                        "daScript/src/simulate/debug_print.cpp",
                        "daScript/src/simulate/json_print.cpp",
                        "daScript/src/simulate/bin_serializer.cpp",
                        "daScript/src/simulate/fs_file_info.cpp"
                    )
                    // test src
                    .AddCppFiles(
                        "daScript/test/test_handles.cpp",
                        "daScript/test/test_enum.cpp"
                    );
            });
    }
}