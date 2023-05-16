#include "annotation_module.hpp"
#include "utils/make_zeroed.hpp"
#include "containers/text.hpp"

#define TUTORIAL_NAME   u8"/scripts/daSTestAnnotation/annotation.das"

int tutorial () {
    // output stream for all compiler messages (stdout. for stringstream use TextWriter)
    auto tout_desc = make_zeroed<skr::das::TextPrinterDescriptor>();
    auto tout = skr::das::TextPrinter::Create(tout_desc);

    // module group for compiled program
    auto lib_desc = make_zeroed<skr::das::LibraryDescriptor>();
    auto library = skr::das::Library::Create(lib_desc);

    // default file access
    auto faccess_desc = make_zeroed<skr::das::FileAccessDescriptor>();
    auto faccess = skr::das::FileAccess::Create(faccess_desc);

    skr::das::CompileDescriptor policies;                        // compiler setup
#ifdef AOT
    // policies.aot = true;
#endif
    // compile program
    skr::text::text script_path = skr::das::Environment::GetRootDir();
    script_path += TUTORIAL_NAME;
    auto program = skr::das::Environment::compile_dascript(
        script_path.u8_str(), faccess, tout, library, &policies);
    if (!program) return -1;
    
    // create context
    auto ctx_desc = make_zeroed<skr::das::ContextDescriptor>();
    ctx_desc.stack_size = program->get_ctx_stack_size();
    auto ctx = skr::das::Context::Create(ctx_desc);
    if ( !program->simulate(ctx, tout) ) return -2;

    // find function. its up to application to check, if function is not null
    auto function = ctx->find_function(u8"test");
    if ( !function ) 
    {
        tout->print(u8"function 'test' not found\n");
        return -3;
    }

    // evaluate 'test' function in the context
    ctx->eval_with_catch(function, nullptr, nullptr);
    
    skr::das::Context::Free(ctx);
    skr::das::Program::Free(program);
    skr::das::Library::Free(library);
    skr::das::TextPrinter::Free(tout);
    skr::das::FileAccess::Free(faccess);
    return 0;
}

int main( int argc, char **argv ) 
{
    // Initialize modules
    auto env_desc = make_zeroed<skr::das::EnvironmentDescriptor>();
    env_desc.argc = argc;
    env_desc.argv = argv;
    skr::das::Environment::Initialize(env_desc);
    int ret = 0;
    {
        // add custom annotation
        ModuleRegister_TestAnnotation regMod = {}; (void)regMod;
        // run the tutorial
        ret = tutorial();
    }
    // shut-down daScript, free all memory
    skr::das::Environment::Finalize();
    return ret;
}
