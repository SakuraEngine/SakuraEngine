#include "HLSLTranslator.hpp"

namespace skr::CppSL
{

HLSLRewritterFrontendAction::HLSLRewritterFrontendAction()
    : clang::ASTFrontendAction()
{

}

bool HLSLRewritterFrontendAction::BeginInvocation(clang::CompilerInstance& CI)
{
    return clang::ASTFrontendAction::BeginInvocation(CI);
}

void HLSLTranslator::HandleTranslationUnit(clang::ASTContext &Context)
{
    // Context.getTranslationUnitDecl()->dump();
}

std::unique_ptr<clang::ASTConsumer> HLSLRewritterFrontendAction::CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef InFile)
{
    auto& LO = CI.getLangOpts();
    LO.CommentOpts.ParseAllComments = false;
    LO.LangStd = clang::LangStandard::lang_hlsl202y;
    return std::make_unique<skr::CppSL::HLSLTranslator>();
}

} // namespace skr::CppSL