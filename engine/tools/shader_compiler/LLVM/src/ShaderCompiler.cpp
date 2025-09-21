#include <optional>
#include <fstream>

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Frontend/CompilerInstance.h>

#include "CppSL/ShaderCompiler.hpp"
#include "CppSL/CppSLAST.hpp"
#include "CppSL/langs/HLSLGenerator.hpp"
#include "CppSL/langs/MSLGenerator.hpp"

#include "ShaderTranslator.hpp"

namespace skr::CppSL {

using namespace clang::tooling;
static llvm::cl::OptionCategory ToolOptionsCategory = llvm::cl::OptionCategory("SSL compiler options");   

template <typename T>
std::unique_ptr<FrontendActionFactory> newFrontendActionFactory2(skr::CppSL::ASTCollection& ASTs) 
{
  class SimpleFrontendActionFactory : public FrontendActionFactory 
  {
  public:
    SimpleFrontendActionFactory(skr::CppSL::ASTCollection& ASTs) : ASTs(ASTs) {}

    std::unique_ptr<clang::FrontendAction> create() override {
      return std::make_unique<T>(ASTs);
    }
    skr::CppSL::ASTCollection& ASTs;
  };

  return std::unique_ptr<FrontendActionFactory>(new SimpleFrontendActionFactory(ASTs));
}

struct ShaderCompilerImpl : public ShaderCompiler
{
public:
    ShaderCompilerImpl(int argc, const char **argv)
        : ASTs()
    {
        auto ExpectedParser = CommonOptionsParser::create(argc, argv, ToolOptionsCategory);
        if (!ExpectedParser)
            llvm::errs() << ExpectedParser.takeError();
        OptionsParser = std::move(ExpectedParser.get());
        tool.emplace(ClangTool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList()));
    }

    int Run() override
    {
        auto factory = newFrontendActionFactory2<CompileFrontendAction>(ASTs);
        auto result = tool->run(factory.get());
#ifdef __APPLE__
        GenerateMSLCode();
#else
        GenerateHLSLCode();
#endif
        return result;
    }

    void GenerateMSLCode()
    {
        for (auto& [kernel, AST] : ASTs.ASTs)
        {
            skr::CppSL::SourceBuilderNew sb;
            skr::CppSL::MSL::MSLGenerator msl_generator;
            auto msl_code = msl_generator.generate_code(sb, AST);
            auto SourceFile = GetSourceFile();
            auto SourceName = SourceFile.filename().replace_extension(".");
            auto PermutationID = ASTs.permutation_id;
            if (!PermutationID.empty())
                PermutationID += "#";
            auto func = ASTs.Kernels[kernel];
            {
                std::wstring target_string = L"";
                std::wstring output_file = L"";
                if (auto stage = func->stage(); stage != skr::CppSL::ShaderStage::None)
                {
                    bool unsupported = false;
                    switch (stage)
                    {
                        case skr::CppSL::ShaderStage::Vertex:
                            target_string = L".vs.";
                            break;
                        case skr::CppSL::ShaderStage::Fragment:
                            target_string = L".fs.";
                            break;
                        case skr::CppSL::ShaderStage::Compute:
                            target_string = L".cs.";
                            break;
                        default:
                            unsupported = true;
                            target_string = L".lib.";
                            break;
                    }
                    if (!unsupported)
                    {
                        std::filesystem::path output_path = std::filesystem::path(PermutationID);
                        output_path += SourceName.wstring() + func->name() + target_string + L"metal";

                        std::wofstream msl_file(output_path);
                        msl_file << msl_code;
                        msl_file.close();
                    }
                }
            }
        }
    }
    
    void GenerateHLSLCode()
    {
        for (auto& [kernel, AST] : ASTs.ASTs)
        {
            skr::CppSL::SourceBuilderNew sb;
            skr::CppSL::HLSL::HLSLGenerator hlsl_generator;
            auto hlsl_code = hlsl_generator.generate_code(sb, AST);
            auto SourceFile = GetSourceFile();
            auto SourceName = SourceFile.filename().replace_extension(".");
            auto PermutationID = ASTs.permutation_id;
            if (!PermutationID.empty())
                PermutationID += "#";
            auto func = ASTs.Kernels[kernel];
            {
                std::wstring target_string = L"";
                std::wstring define_string = L"";
                std::wstring output_file = L"";
                if (auto stage = func->stage(); stage != skr::CppSL::ShaderStage::None)
                {
                    switch (stage)
                    {
                        case skr::CppSL::ShaderStage::Vertex:
                            target_string = L".vs_6_6.";
                            define_string = L"#define CPPSL_VERTEX 1\n";
                            break;
                        case skr::CppSL::ShaderStage::Fragment:
                            target_string = L".ps_6_6.";
                            define_string = L"#define CPPSL_FRAGMENT 1\n";
                            break;
                        case skr::CppSL::ShaderStage::Compute:
                            target_string = L".cs_6_6.";
                            define_string = L"#define CPPSL_COMPUTE 1\n";
                            break;
                        default:
                            target_string = L".lib_6_6.";
                            break;
                    }
                    std::filesystem::path output_path = std::filesystem::path(PermutationID);
                    output_path += SourceName.wstring() + func->name() + target_string + L"hlsl";
                    
                    std::wofstream hlsl_file(output_path);
                    hlsl_file << define_string << hlsl_code;
                    hlsl_file.close();
                }
            }
        }
    }

    std::filesystem::path GetSourceFile() const override
    {
        if (OptionsParser)
        {
            return OptionsParser->getSourcePathList().front();
        }
        return {};
    }

    void SetPermutationId(std::string_view permutation_id) override;

private:
    std::optional<CommonOptionsParser> OptionsParser;
    std::optional<ClangTool> tool;
    ASTCollection ASTs;
};

void ShaderCompilerImpl::SetPermutationId(std::string_view permutation_id)
{
    ASTs.permutation_id = permutation_id;
}

ShaderCompiler* ShaderCompiler::Create(int argc, const char **argv)
{
    return new ShaderCompilerImpl(argc, argv);
}

void ShaderCompiler::Destroy(ShaderCompiler* compiler)
{
    delete compiler;
}

} // namespace skr::CppSL