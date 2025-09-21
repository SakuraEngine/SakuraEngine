#pragma once
#include "CppLikeShaderGenerator.hpp"
#include <unordered_set>

namespace skr::CppSL::MSL
{
struct MSLGenerator : public CppLikeShaderGenerator
{
    MSLGenerator();
    
protected:
    String GetTypeName(const TypeDecl* type) override;
    String GetFunctionName(const FunctionDecl* func) override;
    void VisitAccessExpr(SourceBuilderNew& sb, const AccessExpr* expr) override;
    void VisitBinaryExpr(SourceBuilderNew& sb, const BinaryExpr* expr) override;
    void VisitConstructExpr(SourceBuilderNew& sb, const ConstructExpr* expr) override;
    void VisitDeclRef(SourceBuilderNew& sb, const DeclRefExpr* expr) override;
    void RecordBuiltinHeader(SourceBuilderNew& sb, const AST& ast) override;
    void VisitShaderResource(SourceBuilderNew& sb, const skr::CppSL::VarDecl* var) override;
    void VisitVariable(SourceBuilderNew& sb, const skr::CppSL::VarDecl* var) override;
    void VisitParameter(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl, const skr::CppSL::ParamVarDecl* param) override;
    void VisitField(SourceBuilderNew& sb, const skr::CppSL::TypeDecl* type, const skr::CppSL::FieldDecl* field) override;
    bool SupportConstructor() const override;
    void BeforeGenerateFunctionImplementations(SourceBuilderNew& sb, const AST& ast) override;
    void BeforeGenerateCallArgs(SourceBuilderNew& sb, const skr::CppSL::CallExpr* call) override;
    void BeforeGenerateParamters(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl) override;
    void GenerateFunctionAttributes(SourceBuilderNew& sb, const FunctionDecl* func) override;
    
    bool HasWaveIntrins(const skr::CppSL::FunctionDecl* funcDecl) const;
    const std::unordered_set<const skr::CppSL::FunctionDecl*>& GetFunctionsWithWaveIntrins() const;

private:
    void GenerateSRTs(SourceBuilderNew& sb, const AST& ast);
    void GenerateKernelWrapper(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl);
    void AnalyzeFunctions(const AST& ast);
    void AnalyzeFunction(const skr::CppSL::FunctionDecl* funcDecl);
    void WalkStmt(const skr::CppSL::Stmt* stmt, const skr::CppSL::FunctionDecl* currentFunc);
    
    std::unordered_map<const skr::CppSL::VarDecl*, uint32_t> set_of_vars;
    skr::CppSL::StructureTypeDecl* ctx_type = nullptr;
    
    std::unordered_set<const skr::CppSL::FunctionDecl*> analyzed_functions;
    std::unordered_set<const skr::CppSL::FunctionDecl*> functions_with_wave_intrins;
};
}