#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include "CppSL/CppSLAST.hpp"
#include "clang/Frontend/MultiplexConsumer.h"

namespace skr::CppSL {

struct KernelTranslator;

struct ASTCollection
{
    ASTCollection() = default;
    ASTCollection(const ASTCollection&) = delete;
    ASTCollection& operator=(const ASTCollection&) = delete;

    std::string permutation_id;
    std::map<const clang::FunctionDecl*, AST> ASTs;
    std::map<const clang::FunctionDecl*, const CppSL::FunctionDecl*> Kernels;
};

struct CompileFrontendAction : public clang::ASTFrontendAction 
{
    bool BeginInvocation(clang::CompilerInstance &CI) override;

    CompileFrontendAction(skr::CppSL::ASTCollection& ASTs);
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef InFile) final;
    skr::CppSL::ASTCollection& ASTs;
};

struct FunctionStack
{
public:
    const skr::CppSL::TypeDecl* methodThisType() const;

    std::map<const clang::Expr*, skr::CppSL::Expr*> _member_redirects;
    skr::CppSL::Expr* _this_redirect = nullptr;
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
    friend class KernelTranslator;
    FunctionStack(const clang::FunctionDecl* func, const skr::CppSL::KernelTranslator* pKernelTranslator)
        : func(func), pKernelTranslator(pKernelTranslator)
    {

    }

    const clang::FunctionDecl* func = nullptr;
    const skr::CppSL::KernelTranslator* pKernelTranslator = nullptr;
    FunctionStack* prev = nullptr;
};

class KernelTranslator : public clang::RecursiveASTVisitor<KernelTranslator>
{
public:
    friend struct FunctionStack;
    explicit KernelTranslator(skr::CppSL::AST& AST);
    ~KernelTranslator();

    CppSL::FunctionDecl* Run(clang::ASTContext& Context, const clang::FunctionDecl* stage);

public:
    // ASTVisitor APIs
    bool shouldVisitTemplateInstantiations() const { return true; }
    bool VisitEnumDecl(const clang::EnumDecl* x);
    bool VisitRecordDecl(const clang::RecordDecl* x);
    bool VisitNamespaceDecl(const clang::NamespaceDecl* x);
    bool VisitFunctionDecl(const clang::FunctionDecl* x);
    bool VisitFieldDecl(const clang::FieldDecl* x);
    bool VisitVarDecl(const clang::VarDecl* x);
    bool VisitLambdaExpr(clang::LambdaExpr* lambda) {
        _lambda_map[lambda->getLambdaClass()] = lambda;
        return true;
    }

protected:
    CppSL::TypeDecl* TranslateType(clang::QualType type);
    CppSL::TypeDecl* TranslateRecordDecl(const clang::RecordDecl* x);
    CppSL::TypeDecl* TranslateEnumDecl(const clang::EnumDecl* x);
    CppSL::NamespaceDecl* TranslateNamespaceDecl(const clang::NamespaceDecl* x);
    void AssignDeclsToNamespaces();
    const clang::NamespaceDecl* GetDeclNamespace(const clang::Decl* decl) const;
    CppSL::ParamVarDecl* TranslateParam(std::vector<CppSL::ParamVarDecl*>& params, skr::CppSL::EVariableQualifier qualifier, clang::QualType type, const skr::CppSL::Name& name);
    void TranslateParams(std::vector<CppSL::ParamVarDecl*>& params, const clang::FunctionDecl* x);
    bool TranslateStageEntry(const clang::FunctionDecl* x);
    CppSL::FunctionDecl* TranslateFunction(const clang::FunctionDecl* x, llvm::StringRef override_name = {});
    CppSL::GlobalVarDecl* TranslateGlobalVariable(const clang::VarDecl* x);
    CppSL::Stmt* TranslateCall(const clang::Decl* toCall, const clang::Stmt* callExpr);

    const CppSL::TypeDecl* TranslateLambda(const clang::LambdaExpr* x);
    void TranslateLambdaCapturesToParams(const clang::LambdaExpr* x);

    Stmt* TranslateStmt(const clang::Stmt *x);
    template <typename T>
    T* TranslateStmt(const clang::Stmt* x)
    {
        return (T*)TranslateStmt(x);
    }

    
    bool addType(clang::QualType type, const skr::CppSL::TypeDecl* decl);
    bool addType(clang::QualType type, skr::CppSL::TypeDecl* decl);
    skr::CppSL::TypeDecl* getType(clang::QualType type) const;
    bool addVar(const clang::VarDecl* var, skr::CppSL::VarDecl* decl);
    skr::CppSL::VarDecl* getVar(const clang::VarDecl* var, bool restrict = true) const;
    bool addFunc(const clang::FunctionDecl* func, skr::CppSL::FunctionDecl* decl);
    skr::CppSL::FunctionDecl* getFunc(const clang::FunctionDecl* func) const;
    
    clang::ASTContext* pASTContext = nullptr;
    std::map<const clang::CXXRecordDecl*, const clang::LambdaExpr*> _lambda_map;
    std::vector<const clang::FunctionDecl*> _noignore_funcs;
    std::map<const clang::TagDecl*, skr::CppSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::CppSL::TypeDecl*> _builtin_types;
    std::map<const clang::VarDecl*, skr::CppSL::VarDecl*> _vars;
    std::map<const clang::FunctionDecl*, skr::CppSL::FunctionDecl*> _funcs;
    std::map<const clang::EnumConstantDecl*, skr::CppSL::GlobalVarDecl*> _enum_constants;
    std::map<const clang::NamespaceDecl*, skr::CppSL::NamespaceDecl*> _namespaces;

    const skr::CppSL::TypeDecl* _lambda_proxy = nullptr;

    uint64_t next_lambda_id = 0;
    uint64_t next_template_spec_id = 0;
    uint64_t next_anonymous_id = 0;
    std::map<std::string, std::vector<const clang::FunctionDecl*>> func_names_; 
    std::map<std::string, std::vector<const clang::VarDecl*>> var_names_; 
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
    std::string GetVarName(const clang::VarDecl* var);
    std::string GetFunctionName(const clang::FunctionDecl* func);

    void DumpWithLocation(const clang::Stmt *stmt) const;
    void DumpWithLocation(const clang::Decl *decl) const;
    void ReportFatalError(const std::string& message) const;

    template <typename... Args>
    void ReportFatalError(std::format_string<Args...> _fmt, Args&&... args) const
    {
        auto message = std::format(_fmt, std::forward<Args>(args)...);
        llvm::report_fatal_error(message.c_str());
    }

    template <typename... Args>
    void ReportFatalError(const clang::Stmt* expr, std::format_string<Args...> _fmt, Args&&... args) const
    {
        DumpWithLocation(expr);
        ReportFatalError(_fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void ReportFatalError(const clang::Decl* decl, std::format_string<Args...> _fmt, Args&&... args) const
    {
        DumpWithLocation(decl);
        ReportFatalError(_fmt, std::forward<Args>(args)...);
    }
    
    std::map<std::string, skr::CppSL::BinaryOp> _bin_ops;
    
    const bool kUseNamespace = false;
    const bool kCollectUsedResourcesOnly = false;
};

class ShaderTranslator : public clang::ASTConsumer, public clang::RecursiveASTVisitor<ShaderTranslator>
{
public:
    explicit ShaderTranslator(skr::CppSL::ASTCollection& ASTs);
    virtual ~ShaderTranslator() override;
    void HandleTranslationUnit(clang::ASTContext &Context) override;
    bool VisitFunctionDecl(const clang::FunctionDecl* x);

private:
    std::vector<const clang::FunctionDecl*> _stages;
    ASTCollection& ASTs;
};
    
} // namespace skr::CppSL