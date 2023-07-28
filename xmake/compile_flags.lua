target("SkrCompileFlags")
    set_kind("headeronly")
        on_config(function(target)
            local is_clang = target:toolchain("clang") or target:toolchain("clang-cl")
            local is_msvc = target:toolchain("msvc") or target:toolchain("clang-cl")
            local project_cxflags = {}
            local project_ldflags = {}

            -- uses utf-8 charset at runtime
            if target:is_plat("windows") then
                table.insert(project_cxflags, "/execution-charset:utf-8")
                table.insert(project_cxflags, "/source-charset:utf-8")
            end

            if is_clang then
                -- table.insert(project_cxflags, "-Werror")
                -- table.insert(project_cxflags, "-Wno-invalid-offsetof")
                table.insert(project_cxflags, "-Wno-unused-private-field")
                table.insert(project_cxflags, "-Wno-deprecated-builtins")
                table.insert(project_cxflags, "-Wno-pragma-system-header-outside-header")
                table.insert(project_cxflags, "-Wno-ambiguous-reversed-operator")
                table.insert(project_cxflags, "-Wno-unused-command-line-argument")
                table.insert(project_cxflags, "-Wno-format")
                -- table.insert(project_cxflags, "-Wno-deprecated-builtins")
                table.insert(project_cxflags, "-Wno-switch")
                table.insert(project_cxflags, "-Wno-misleading-indentation")
                table.insert(project_cxflags, "-Wno-unknown-pragmas")
                table.insert(project_cxflags, "-Wno-unused-function")
                table.insert(project_cxflags, "-Wno-ignored-attributes")
                table.insert(project_cxflags, "-Wno-deprecated-declarations")
                table.insert(project_cxflags, "-Wno-nullability-completeness")
                table.insert(project_cxflags, "-Wno-tautological-undefined-compare")
                table.insert(project_cxflags, "-Werror=return-type")
                -- enable time trace with clang compiler
                table.insert(project_cxflags, "-ftime-trace")
                if is_msvc then
                    table.insert(project_cxflags, "-Wno-microsoft-cast")
                    table.insert(project_cxflags, "-Wno-microsoft-include")
                    table.insert(project_cxflags, "-Wno-microsoft-enum-forward-reference")
                    if (is_mode("asan")) then
                        table.insert(project_ldflags, "-fsanitize=address")
                    end
                end
            end

            if is_msvc then
                table.insert(project_ldflags, "/IGNORE:4217,4286")
                table.insert(project_cxflags, "/Zc:__cplusplus")
                table.insert(project_cxflags, "/FC")
                table.insert(project_cxflags, "/GR-")
                table.insert(project_cxflags, "/wd4251")
                if (is_mode("asan")) then
                    table.insert(project_ldflags, "/fsanitize=address")
                end
            end

            target:add("cxflags", project_cxflags, {public = true})
            if target:is_plat("macosx") then
                target:add("mxflags", project_cxflags, {public = true})
            end
            target:add("ldflags", project_ldflags, {public = true})
            target:add("shflags", project_ldflags, {public = true})
        end)