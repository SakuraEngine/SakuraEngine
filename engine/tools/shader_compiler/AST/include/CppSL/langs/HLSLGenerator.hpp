#pragma once
#include "CppLikeShaderGenerator.hpp"
#include <unordered_map>

namespace skr::CppSL::HLSL
{
struct HLSLGenerator : public CppLikeShaderGenerator
{
public:
    String GetTypeName(const TypeDecl* type) override;
    void VisitBinaryExpr(SourceBuilderNew& sb, const BinaryExpr* binary) override;
    void VisitConstructExpr(SourceBuilderNew& sb, const ConstructExpr* expr) override;
    void RecordBuiltinHeader(SourceBuilderNew& sb, const AST& ast) override;
    void VisitAccessExpr(SourceBuilderNew& sb, const AccessExpr* expr) override;
    void VisitShaderResource(SourceBuilderNew& sb, const skr::CppSL::VarDecl* var) override;
    void VisitVariable(SourceBuilderNew& sb, const skr::CppSL::VarDecl* var) override;
    void VisitParameter(SourceBuilderNew& sb, const skr::CppSL::FunctionDecl* funcDecl, const skr::CppSL::ParamVarDecl* param) override;
    void VisitField(SourceBuilderNew& sb, const skr::CppSL::TypeDecl* type, const skr::CppSL::FieldDecl* field) override;
    void VisitConstructor(SourceBuilderNew& sb, const ConstructorDecl* ctor, FunctionStyle style) override;
    void GenerateStmtAttributes(SourceBuilderNew& sb, const skr::CppSL::Stmt* stmt) override;
    void GenerateFunctionAttributes(SourceBuilderNew& sb, const FunctionDecl* func) override;
    void GenerateFunctionSignaturePostfix(SourceBuilderNew& sb, const FunctionDecl* func) override;
    bool SupportConstructor() const override;
    bool HLSL_RemoveEmptyResourceInit(const ConstructorDecl::MemberInit* init) const override;
    
private:
    void GenerateArrayHelpers(SourceBuilderNew& sb, const AST& ast);
};
} // namespace skr::CppSL::HLSL