#pragma once
#include "CppSL/CppSLAST.hpp"
#include "CppSL/SourceBuilder.hpp"

namespace skr::CppSL {

struct ASTDumper
{
public:
    void Visit(const skr::CppSL::Decl* decl, SourceBuilderNew& sb);
    void Visit(const skr::CppSL::Stmt* stmt, SourceBuilderNew& sb);

protected:
    void visit(const skr::CppSL::Stmt* stmt, SourceBuilderNew& sb);
    void visit(const skr::CppSL::TypeDecl* typeDecl, SourceBuilderNew& sb);
    void visit(const skr::CppSL::FieldDecl* fieldDecl, SourceBuilderNew& sb);
    void visit(const skr::CppSL::ParamVarDecl* paramDecl, SourceBuilderNew& sb);
    void visit(const skr::CppSL::FunctionDecl* funcDecl, SourceBuilderNew& sb);
    void visit(const skr::CppSL::VarDecl* varDecl, SourceBuilderNew& sb);
};

} // namespace skr::CppSL