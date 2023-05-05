#include "types.hpp"

inline static void __das_initialize() { NEED_ALL_DEFAULT_MODULES; }

namespace skr {
namespace das {

Environment::~Environment() SKR_NOEXCEPT
{
    
}

void Environment::Initialize(const EnvironmentDescriptor &desc) SKR_NOEXCEPT
{
    ::__das_initialize();
    ::das::Module::Initialize(); 
}

void Environment::Finalize() SKR_NOEXCEPT
{
    ::das::Module::Shutdown();
}

Program* Environment::compile_dascript(const char8_t* name, FileAccess* faccess, TextPrinter* tout, Library* lib)
{
    auto fAcess = static_cast<FileAccessImpl*>(faccess);
    auto tOut = static_cast<TextPrinterImpl*>(tout);
    auto Lib = static_cast<LibraryImpl*>(lib);
    auto program = compileDaScript(
        (const char*)name, 
        fAcess->fAccess, tOut->printer,
        Lib->libGroup);
    if ( program->failed() ) {
        // if compilation failed, report errors
        tOut->printer << "failed to compile\n";
        for ( auto & err : program->errors ) {
            tOut->printer << reportError(err.at, err.what, err.extra, err.fixme, err.cerr );
        }
        return nullptr;
    }
    return SkrNew<ProgramImpl>(::das::move(program));
}

} // namespace das
} // namespace skr