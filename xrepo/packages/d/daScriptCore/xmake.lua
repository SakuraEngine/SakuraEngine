package("daScriptCore")
    set_homepage("https://dascript.org/")
    set_description("daScript - high-performance statically strong typed scripting language")

    add_versions("2023.4.25-skr.10", "4ffaed3f7b6a31681008ede702d5b62c1d7d50cf02fcbd1fce23654a1b315da7")

    add_defines("URI_STATIC_BUILD")
    add_defines("URIPARSER_BUILD_CHAR")
    on_load(function (package)
        if package:is_debug() then
            package:add("defines", "DAS_SMART_PTR_TRACKER=1")
            package:add("defines", "DAS_SMART_PTR_MAGIC=1")
        else 
            package:add("defines", "DAS_DEBUGGER=0")
            package:add("defines", "DAS_SMART_PTR_TRACKER=0")
            package:add("defines", "DAS_SMART_PTR_MAGIC=0")
            package:add("defines", "DAS_FREE_LIST=1")
            package:add("defines", "DAS_FUSION=2")
        end
    end)

    on_install(function (package)
        os.mkdir(package:installdir())
        os.cp(path.join(package:scriptdir(), "port", "daScript", "include"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "src"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "test"), ".")
        os.cp(path.join(package:scriptdir(), "port", "daScript", "3rdparty"), ".")
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")

        local configs = {}
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include "daScript/daScript.h"
            using namespace das;
            const char * tutorial_text = R""""(
            [export]
            def test
                print("this is nano tutorial\n")
            )"""";
            int main( int, char * [] ) {
                // request all da-script built in modules
                NEED_ALL_DEFAULT_MODULES;
                // Initialize modules
                Module::Initialize();
                // make file access, introduce string as if it was a file
                auto fAccess = make_smart<FsFileAccess>();
                auto fileInfo = make_unique<TextFileInfo>(tutorial_text, uint32_t(strlen(tutorial_text)), false);
                fAccess->setFileInfo("dummy.das", das::move(fileInfo));
                // compile script
                TextPrinter tout;
                ModuleGroup dummyLibGroup;
                auto program = compileDaScript("dummy.das", fAccess, tout, dummyLibGroup);
                if ( program->failed() ) return -1;
                // create context
                Context ctx(program->getContextStackSize());
                if ( !program->simulate(ctx, tout) ) return -2;
                // find function. its up to application to check, if function is not null
                auto function = ctx.findFunction("test");
                if ( !function ) return -3;
                // call context function
                ctx.evalWithCatch(function, nullptr);
                // shut-down daScript, free all memory
                Module::Shutdown();
                return 0;
            }
        ]]}, {configs = {languages = "c++17"}}))
    end)


