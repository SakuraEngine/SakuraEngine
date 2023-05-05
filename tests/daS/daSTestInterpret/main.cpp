#include "SkrDAScript/text_printer.hpp"
#include "SkrDAScript/ctx.hpp"
#include "utils/make_zeroed.hpp"

using namespace das;

const char8_t* tutorial_text = u8R""""(
[export]
def test
    print("this is nano tutorial\n")
)"""";

int main( int, char * [] ) {
    auto env_desc = make_zeroed<skr::das::EnvironmentDescriptor>();
    skr::das::Environment::Initialize(env_desc);
    // make file access, introduce string as if it was a file
    auto facess_desc = make_zeroed<skr::das::FileAccessDescriptor>();
    auto facess = skr::das::FileAccess::Create(facess_desc);
    facess->set_text_file(u8"dummy.das", tutorial_text, uint32_t(strlen((const char*)tutorial_text)));
    // compile script
    auto tout_desc = make_zeroed<skr::das::TextPrinterDescriptor>();
    auto tout = skr::das::TextPrinter::Create(tout_desc);
    /*
    ModuleGroup dummyLibGroup;
    auto program = compileDaScript("dummy.das", faccess, tout, dummyLibGroup);
    if ( program->failed() ) return -1;
    // create context
    Context ctx(program->getContextStackSize());
    if ( !program->simulate(ctx, tout) ) return -2;
    // find function. its up to application to check, if function is not null
    auto function = ctx.findFunction("test");
    if ( !function ) return -3;
    // call context function
    ctx.evalWithCatch(function, nullptr);
    */
    skr::das::TextPrinter::Free(tout);
    skr::das::FileAccess::Free(facess);
    skr::das::Environment::Finalize();
    return 0;
}