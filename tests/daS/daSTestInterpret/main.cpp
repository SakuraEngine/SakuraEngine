#include "SkrDAScript/daScript.hpp"
#include "misc/make_zeroed.hpp"
#include "containers/string.hpp"

using namespace das;

const char8_t* tutorial_text = u8R"(
var p : int = 12
[export]
def test
    print("this is nano tutorial\n")
    p = p + 1
    return p
)";

int main(int argc, char** argv) {
    auto env_desc = make_zeroed<skr::das::EnvironmentDescriptor>();
    env_desc.argc = argc;
    env_desc.argv = argv;
    skr::das::Environment::Initialize(env_desc);
    // make file access, introduce string as if it was a file
    auto faccess_desc = make_zeroed<skr::das::FileAccessDescriptor>();
    auto faccess = skr::das::FileAccess::Create(faccess_desc);
    faccess->set_text_file(u8"dummy.das", tutorial_text, uint32_t(strlen((const char*)tutorial_text)));
    
    // output stream for all compiler messages (stdout. for stringstream use TextWriter)
    auto tout_desc = make_zeroed<skr::das::TextPrinterDescriptor>();
    auto tout = skr::das::TextPrinter::Create(tout_desc);

    // module group for compiled program
    auto lib_desc = make_zeroed<skr::das::LibraryDescriptor>();
    auto library = skr::das::Library::Create(lib_desc);

    // compile script
    auto program = skr::das::Environment::compile_dascript(u8"dummy.das", faccess, tout, library);
    if (!program) return -1;
    
    // create context
    auto ctx_desc = make_zeroed<skr::das::ContextDescriptor>();
    ctx_desc.stack_size = program->get_ctx_stack_size();
    auto ctx = skr::das::Context::Create(ctx_desc);
    if ( !program->simulate(ctx, tout) ) return -2;

    // find function. its up to application to check, if function is not null
    auto function = ctx->find_function(u8"test");
    if ( !function ) return -3;

    // call context function
    const auto res = ctx->eval(function);
    const auto message = skr::format(u8"{}\n", res.load<int32_t>());
    tout->print(message.u8_str());

    skr::das::Context::Free(ctx);
    skr::das::Program::Free(program);
    skr::das::Library::Free(library);
    skr::das::TextPrinter::Free(tout);
    skr::das::FileAccess::Free(faccess);
    skr::das::Environment::Finalize();
    return 0;
}