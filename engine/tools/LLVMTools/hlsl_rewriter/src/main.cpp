#include "HLSLTranslator.hpp"
#include <clang/Tooling/CommonOptionsParser.h>

using namespace skr::CppSL;
static llvm::cl::OptionCategory ToolOptionsCategory = llvm::cl::OptionCategory("HLSLRewritter compiler options");   

int main(int argc, const char **argv)
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i)
    {
        auto arg = std::string_view(argv[i]);
        args.emplace_back(argv[i]);
    }
    args.emplace_back("--");
    args.emplace_back("--driver-mode=dxc");
    args.emplace_back("-ID:\\Code\\D5Engine\\engine\\tools\\LLVMTools\\hlsl_rewriter\\src");
    args.emplace_back("-Tlib_6_6");
    args.emplace_back("-fcolor-diagnostics");
    args.emplace_back("-fno-crash-diagnostics");
    args.emplace_back("-Wno-invalid-constexpr");

    std::vector<const char*> args_ptr(args.size());
    for (size_t i = 0; i < args.size(); ++i)
    {
        args_ptr[i] = args[i].c_str();
    }
    int N = args_ptr.size();
    auto factory = clang::tooling::newFrontendActionFactory<HLSLRewritterFrontendAction>();
    auto parser = clang::tooling::CommonOptionsParser::create(N, args_ptr.data(), ToolOptionsCategory);
    if (!parser)
    {
        llvm::errs() << parser.takeError();
    }
    auto tool = clang::tooling::ClangTool(parser->getCompilations(), parser->getSourcePathList());
    return tool.run(factory.get());
}