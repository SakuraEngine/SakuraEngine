#include <optional>
#include <fstream>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/CompilerInstance.h>

#include "CppSL/ShaderCompiler.hpp"
#include "CppSL/AST.hpp"
#include "CppSL/langs/HLSLGenerator.hpp"

#include "ShaderASTConsumer.hpp"

namespace skr::CppSL {

using namespace clang::tooling;
static llvm::cl::OptionCategory ToolOptionsCategory = llvm::cl::OptionCategory("SSL compiler options");   

template <typename T>
std::unique_ptr<FrontendActionFactory> newFrontendActionFactory2(skr::CppSL::AST& AST) 
{
  class SimpleFrontendActionFactory : public FrontendActionFactory 
  {
  public:
    SimpleFrontendActionFactory(skr::CppSL::AST& AST) : AST(AST) {}

    std::unique_ptr<clang::FrontendAction> create() override {
      return std::make_unique<T>(AST);
    }
    skr::CppSL::AST& AST;
  };

  return std::unique_ptr<FrontendActionFactory>(new SimpleFrontendActionFactory(AST));
}

struct ShaderCompilerImpl : public ShaderCompiler
{
public:
    ShaderCompilerImpl(int argc, const char **argv)
    {
        auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolOptionsCategory);
        if (!ExpectedParser)
            llvm::errs() << ExpectedParser.takeError();
        OptionsParser = std::move(ExpectedParser.get());
        tool.emplace(ClangTool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList()));
    }

    int Run() override
    {
        auto factory = newFrontendActionFactory2<CompileFrontendAction>(AST);
        auto result = tool->run(factory.get());
        GenerateHLSLCode();
        return result;
    }

    void GenerateHLSLCode()
    {
        skr::CppSL::SourceBuilderNew sb;
        skr::CppSL::HLSLGenerator hlsl_generator;
        auto hlsl_code = hlsl_generator.generate_code(sb, AST);
        auto SourceFile = GetSourceFile();
        auto SourceName = SourceFile.filename().replace_extension(".");
        for (auto func : GetAST().funcs())
        {
            std::wstring target_string = L"";
            std::wstring output_file = L"";
            if (auto stage = func->stage(); stage != skr::CppSL::ShaderStage::None)
            {
                switch (stage)
                {
                    case skr::CppSL::ShaderStage::Vertex:
                        target_string = L".vs_6_1.";
                        break;
                    case skr::CppSL::ShaderStage::Fragment:
                        target_string = L".ps_6_1.";
                        break;
                    case skr::CppSL::ShaderStage::Compute:
                        target_string = L".cs_6_1.";
                        break;
                    default:
                        continue;
                }
                std::filesystem::path output_path = SourceName.wstring() + func->name() + target_string + L"hlsl";
                std::wofstream hlsl_file(output_path);
                hlsl_file << hlsl_code;
                hlsl_file.close();
            }
        }
    }

    const AST& GetAST() const override
    {
        return AST;
    }

    std::filesystem::path GetSourceFile() const override
    {
        if (OptionsParser)
        {
            return OptionsParser->getSourcePathList().front();
        }
        return {};
    }

private:
    std::optional<CommonOptionsParser> OptionsParser;
    std::optional<ClangTool> tool;
    skr::CppSL::AST AST;
};

ShaderCompiler* ShaderCompiler::Create(int argc, const char **argv)
{
    return new ShaderCompilerImpl(argc, argv);
}

void ShaderCompiler::Destroy(ShaderCompiler* compiler)
{
    delete compiler;
}

} // namespace skr::CppSL