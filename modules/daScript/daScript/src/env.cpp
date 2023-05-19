#include "types.hpp"

inline static void __das_initialize() 
{ 
    NEED_ALL_DEFAULT_MODULES; 
    NEED_MODULE(Module_UriParser)
    NEED_MODULE(Module_JobQue)
}

namespace skr {
namespace das {

Environment::~Environment() SKR_NOEXCEPT
{
    
}

void Environment::Initialize(const EnvironmentDescriptor &desc) SKR_NOEXCEPT
{
    ::__das_initialize();
    if (desc.argc && desc.argv)
    {
        ::das::setCommandLineArguments(desc.argc, desc.argv);
    }
    ::das::Module::Initialize(); 
}

void Environment::Finalize() SKR_NOEXCEPT
{
    ::das::Module::Shutdown();
}

Program* Environment::compile_dascript(const char8_t* name, FileAccess* faccess, TextPrinter* tout, Library* lib, const CompileDescriptor* desc)
{
    const bool export_all = false;
    auto CodeOfPolicies = ::das::CodeOfPolicies();
    if (desc)
    {
        CodeOfPolicies.aot = desc->aot;
    }
    auto fAcess = static_cast<FileAccessImpl*>(faccess);
    auto tOut = static_cast<TextPrinterImpl*>(tout);
    auto Lib = static_cast<LibraryImpl*>(lib);
    
    auto program = compileDaScript(
        (const char*)name, 
        fAcess->fAccess, tOut->printer,
        Lib->libGroup,
        export_all, CodeOfPolicies);
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

const skr::string Environment::GetRootDir() SKR_NOEXCEPT
{
    const auto r = ::das::getDasRoot();
    return skr::string((const char8_t*)r.c_str());
}

} // namespace das
} // namespace skr