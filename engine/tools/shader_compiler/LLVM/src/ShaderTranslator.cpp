#include "LLVMHelpers.hpp"
#include "ShaderTranslator.hpp"
#include "CppSL/magic_enum/magic_enum.hpp"
#include <filesystem>

namespace skr::CppSL
{

void KernelTranslator::ReportFatalError(const std::string& message) const
{
    llvm::report_fatal_error(message.c_str());
}

void KernelTranslator::DumpWithLocation(const clang::Stmt* stmt) const
{
    stmt->getBeginLoc().dump(pASTContext->getSourceManager());
    stmt->dump();
}

void KernelTranslator::DumpWithLocation(const clang::Decl* decl) const
{
    decl->getBeginLoc().dump(pASTContext->getSourceManager());
    decl->dump();
}

KernelTranslator::KernelTranslator(skr::CppSL::AST& AST)
    : AST(AST)
{
    for (uint32_t i = 0; i < (uint32_t)BinaryOp::COUNT; i++)
    {
        const auto op = (BinaryOp)i;
        _bin_ops.emplace(magic_enum::enum_name(op), op);
    }
}

KernelTranslator::~KernelTranslator()
{
    for (auto& [func, stack] : _stacks)
    {
        delete stack;
    }
}

FunctionStack* KernelTranslator::zzNewStack(const clang::FunctionDecl* func)
{
    auto stack = new FunctionStack(func, this);
    _stacks.emplace(func, stack);
    return stack;
}


bool KernelTranslator::VisitFieldDecl(const clang::FieldDecl* x)
{
    if (IsDump(x))
        x->dump();

    if (!LanguageRule_BanDoubleFieldsAndVariables(x, x->getType()))
        ReportFatalError(x, "Double fields are not allowed");

    return true;
}

bool KernelTranslator::VisitVarDecl(const clang::VarDecl* Var)
{
    if (IsDump(Var))
        Var->dump();

    if (!kCollectUsedResourcesOnly && Var->isFileVarDecl())
    {
        auto _type = getType(Var->getType());
        if (_type->is_resource())
        {
            TranslateGlobalVariable(Var);
        }
    }
    return true;
}

bool KernelTranslator::VisitEnumDecl(const clang::EnumDecl* enumDecl)
{
    TranslateEnumDecl(enumDecl);
    return true;
}

bool KernelTranslator::VisitRecordDecl(const clang::RecordDecl* recordDecl)
{
    TranslateRecordDecl(recordDecl);
    return true;
}

bool KernelTranslator::VisitFunctionDecl(const clang::FunctionDecl* x)
{
    // some necessary functions should never be ignored, so we translate then after the kernel
    if (auto AsNoignore = IsNoIgnore(x))
    {
        _noignore_funcs.emplace_back(x);
    }
    return true;
}

bool KernelTranslator::VisitNamespaceDecl(const clang::NamespaceDecl* namespaceDecl)
{
    TranslateNamespaceDecl(namespaceDecl);
    return true;
}

clang::QualType KernelTranslator::Decay(clang::QualType type) const
{
    auto _type = type.getNonReferenceType()
                     .getUnqualifiedType()
                     .getDesugaredType(*pASTContext)
                     .getCanonicalType();
    return _type;
}

CppSL::FunctionDecl* KernelTranslator::Run(clang::ASTContext& Context, const clang::FunctionDecl* stage)
{
    pASTContext = &Context;

    // add primitive type mappings
    addType(Context.VoidTy, AST.VoidType);
    addType(Context.BoolTy, AST.BoolType);
    addType(Context.FloatTy, AST.FloatType);
    addType(Context.UnsignedIntTy, AST.UIntType);
    addType(Context.IntTy, AST.IntType);
    addType(Context.DoubleTy, AST.DoubleType);
    addType(Context.UnsignedLongTy, AST.U64Type);
    addType(Context.UnsignedLongLongTy, AST.U64Type);
    addType(Context.LongLongTy, AST.I64Type);

    // add record types
    TraverseDecl(Context.getTranslationUnitDecl());

    // translate from stage entries
    TranslateStageEntry(stage);

    AssignDeclsToNamespaces();

    return _funcs[stage];
}

ShaderTranslator::ShaderTranslator(skr::CppSL::ASTCollection& ASTs)
    : clang::ASTConsumer(), ASTs(ASTs)
{

}

ShaderTranslator::~ShaderTranslator()
{

}

bool ShaderTranslator::VisitFunctionDecl(const clang::FunctionDecl* x)
{
    if (auto StageInfo = IsStage(x))
    {
        _stages.emplace_back(x);
    }
    return true;
}

void ShaderTranslator::HandleTranslationUnit(clang::ASTContext& Context)
{
    TraverseDecl(Context.getTranslationUnitDecl());
    for (auto stage : _stages)
    {
        ASTs.ASTs.try_emplace(stage);
    }
    for (auto stage : _stages)
    {
        KernelTranslator Translator(ASTs.ASTs[stage]);
        ASTs.Kernels[stage] = Translator.Run(Context, stage);
    }
}

CompileFrontendAction::CompileFrontendAction(skr::CppSL::ASTCollection& ASTs)
    : clang::ASTFrontendAction()
    , ASTs(ASTs)
{
}

bool CompileFrontendAction::BeginInvocation(clang::CompilerInstance& CI)
{
    clang::DependencyOutputOptions& DepOpts = CI.getInvocation().getDependencyOutputOpts();

    const auto& Input = CI.getFrontendOpts().Inputs[0];

    std::filesystem::path P = Input.getFile().str();
    auto N = P.filename().replace_extension("").string();
    auto PermutationID = ASTs.permutation_id;
    if (PermutationID.empty())
    {
        DepOpts.OutputFile = N + ".d";
    }
    else
    {
        DepOpts.OutputFile = PermutationID + "#" + N + ".d";
    }
    DepOpts.Targets.push_back(N + ".o");
    DepOpts.UsePhonyTargets = false;
    DepOpts.IncludeSystemHeaders = true;

    return clang::ASTFrontendAction::BeginInvocation(CI);
}

std::unique_ptr<clang::ASTConsumer> CompileFrontendAction::CreateASTConsumer(clang::CompilerInstance& CI, llvm::StringRef InFile)
{
    auto& LO = CI.getLangOpts();
    LO.CommentOpts.ParseAllComments = false;
    return std::make_unique<skr::CppSL::ShaderTranslator>(ASTs);
}

} // namespace skr::CppSL