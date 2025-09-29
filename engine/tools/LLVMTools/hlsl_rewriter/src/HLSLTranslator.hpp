#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>

namespace skr::CppSL
{

struct HLSLRewritterFrontendAction : public clang::ASTFrontendAction 
{
    bool BeginInvocation(clang::CompilerInstance &CI) override;

    HLSLRewritterFrontendAction();
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
};

struct HLSLTranslator : public clang::ASTConsumer
{
    void HandleTranslationUnit(clang::ASTContext &Context) override;
};

} // namespace skr::CppSL