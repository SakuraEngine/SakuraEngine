#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "CppSL/AST.hpp"

namespace skr::CppSL {

struct ASTConsumer;

struct CompileFrontendAction : public clang::ASTFrontendAction 
{
public:
    bool BeginInvocation(clang::CompilerInstance &CI) override;

    CompileFrontendAction(skr::CppSL::AST& AST);
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
    skr::CppSL::AST& AST;
};

struct FunctionStack
{
public:
    const skr::CppSL::TypeDecl* methodThisType() const;

    std::map<const clang::Expr*, skr::CppSL::Expr*> _member_redirects;
    skr::CppSL::Expr* _this_redirect;
    std::map<const clang::VarDecl*, skr::CppSL::ParamVarDecl*> _value_redirects;
    std::vector<skr::CppSL::ParamVarDecl*> _captured_params;
    struct CapturedParamInfo
    {
        const clang::LambdaExpr* owner = nullptr;
        const clang::VarDecl* asVar = nullptr;
        const clang::FieldDecl* asCaptureThisField = nullptr;
        bool operator<(const CapturedParamInfo& other) const
        {
            return std::tie(owner, asVar, asCaptureThisField) < std::tie(other.owner, other.asVar, other.asCaptureThisField);
        }
    };
    std::map<skr::CppSL::ParamVarDecl*, CapturedParamInfo> _captured_infos;
    std::map<CapturedParamInfo, skr::CppSL::ParamVarDecl*> _captured_maps;
    std::set<const clang::LambdaExpr*> _local_lambdas;

private:
    friend class ASTConsumer;
    FunctionStack(const clang::FunctionDecl* func, const skr::CppSL::ASTConsumer* pASTConsumer)
        : func(func), pASTConsumer(pASTConsumer)
    {

    }

    const clang::FunctionDecl* func = nullptr;
    const skr::CppSL::ASTConsumer* pASTConsumer = nullptr;
    FunctionStack* prev = nullptr;
};

class ASTConsumer : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ASTConsumer>
{
public:
    friend struct FunctionStack;
    explicit ASTConsumer(skr::CppSL::AST& AST);
    virtual ~ASTConsumer() override;

    void HandleTranslationUnit(clang::ASTContext &Context) override;

public:
    // ASTVisitor APIs
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitEnumDecl(const clang::EnumDecl* x);
    bool VisitRecordDecl(const clang::RecordDecl* x);
    bool VisitFunctionDecl(const clang::FunctionDecl* x);
    bool VisitFieldDecl(const clang::FieldDecl* x);
    bool VisitVarDecl(const clang::VarDecl* x);

protected:
    CppSL::TypeDecl* TranslateType(clang::QualType type);
    CppSL::TypeDecl* TranslateRecordDecl(const clang::RecordDecl* x);
    CppSL::TypeDecl* TranslateEnumDecl(const clang::EnumDecl* x);
    CppSL::ParamVarDecl* TranslateParam(std::vector<CppSL::ParamVarDecl*>& params, skr::CppSL::EVariableQualifier qualifier, const skr::CppSL::TypeDecl* type, const skr::CppSL::Name& name);
    void TranslateParams(std::vector<CppSL::ParamVarDecl*>& params, const clang::FunctionDecl* x);
    CppSL::FunctionDecl* TranslateFunction(const clang::FunctionDecl* x, llvm::StringRef override_name = {});
    const CppSL::TypeDecl* TranslateLambda(const clang::LambdaExpr* x);
    void TranslateLambdaCapturesToParams(const clang::LambdaExpr* x);
    CppSL::GlobalVarDecl* TranslateGlobalVariable(const clang::VarDecl* x);
    CppSL::Stmt* TranslateCall(const clang::Decl* toCall, const clang::Stmt* callExpr);

    Stmt* TranslateStmt(const clang::Stmt *x);
    template <typename T>
    T* TranslateStmt(const clang::Stmt* x);
    
    bool addType(clang::QualType type, const skr::CppSL::TypeDecl* decl);
    bool addType(clang::QualType type, skr::CppSL::TypeDecl* decl);
    skr::CppSL::TypeDecl* getType(clang::QualType type) const;
    bool addVar(const clang::VarDecl* var, skr::CppSL::VarDecl* decl);
    skr::CppSL::VarDecl* getVar(const clang::VarDecl* var) const;
    bool addFunc(const clang::FunctionDecl* func, skr::CppSL::FunctionDecl* decl);
    skr::CppSL::FunctionDecl* getFunc(const clang::FunctionDecl* func) const;
    
    clang::ASTContext* pASTContext = nullptr;
    std::map<const clang::TagDecl*, skr::CppSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::CppSL::TypeDecl*> _builtin_types;
    std::map<const clang::VarDecl*, skr::CppSL::VarDecl*> _vars;
    std::map<const clang::FunctionDecl*, skr::CppSL::FunctionDecl*> _funcs;
    std::map<const clang::EnumConstantDecl*, skr::CppSL::GlobalVarDecl*> _enum_constants;

    std::map<const clang::LambdaExpr*, const skr::CppSL::TypeDecl*> _lambda_types;
    std::map<const clang::CXXMethodDecl*, const clang::LambdaExpr*> _lambda_methods;
    std::map<const skr::CppSL::TypeDecl*, const clang::LambdaExpr*> _lambda_wrappers;

    uint64_t next_lambda_id = 0;
    uint64_t next_template_spec_id = 0;
    AST& AST;
    
protected:
    FunctionStack* root_stack = nullptr;
    FunctionStack* current_stack = nullptr;
    std::map<const clang::FunctionDecl*, FunctionStack*> _stacks;

    FunctionStack* zzNewStack(const clang::FunctionDecl* func);
    void appendStack(const clang::FunctionDecl* func)
    {
        auto prev = current_stack;
        auto _new = zzNewStack(func);
        _new->prev = prev;
        current_stack = _new;
        if (!root_stack) root_stack = _new;
    }
    void popStack()
    {
        if (current_stack)
        {
            current_stack = current_stack->prev;
        }
    }

protected:
    clang::QualType Decay(clang::QualType type) const;
    void CheckStageInputs(const clang::FunctionDecl* x, skr::CppSL::ShaderStage stage);
    void DumpWithLocation(const clang::Stmt *stmt) const;
    void DumpWithLocation(const clang::Decl *decl) const;
    void ReportFatalError(const std::string& message) const;
    template <typename... Args>
    void ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const;
    template <typename... Args>
    void ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const;
    template <typename... Args>
    void ReportFatalError(const clang::Decl* decl, std::format_string<Args...> _fmt, Args&&... args) const;
    
    std::map<std::string, skr::CppSL::BinaryOp> _bin_ops;
};
    
} // namespace skr::CppSL