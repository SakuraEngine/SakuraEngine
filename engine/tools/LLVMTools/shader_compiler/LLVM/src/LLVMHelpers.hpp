#pragma once
#include "CppSL/Stmt.hpp"
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Frontend/CompilerInstance.h>

namespace skr::CppSL
{
class DeferGuard
{
public:
    template <typename F>
    DeferGuard(F&& f)
        : func(std::forward<F>(f))
    {
    }

    ~DeferGuard() { func(); }

private:
    std::function<void()> func;
};

String ToText(clang::StringRef str);
std::string OpKindToName(clang::OverloadedOperatorKind kind);
CppSL::UnaryOp TranslateUnaryOp(clang::UnaryOperatorKind op);
CppSL::BinaryOp TranslateBinaryOp(clang::BinaryOperatorKind op);
clang::AnnotateAttr* ExistShaderAttrWithName(const clang::Decl* decl, const char* name);
const clang::AnnotateAttr* ExistShaderAttrWithName(const clang::AttributedStmt* stmt, const char* name);

inline static clang::AnnotateAttr* IsIgnore(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "ignore"); }
inline static clang::AnnotateAttr* IsNoIgnore(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "noignore"); }
inline static clang::AnnotateAttr* IsBuiltin(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "builtin"); }
inline static clang::AnnotateAttr* IsDump(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "dump"); }
inline static clang::AnnotateAttr* IsKernel(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "kernel"); }
inline static clang::AnnotateAttr* IsSwizzle(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "swizzle"); }
inline static clang::AnnotateAttr* IsUnaOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "unaop"); }
inline static clang::AnnotateAttr* IsBinOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "binop"); }
inline static clang::AnnotateAttr* IsCallOp(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "callop"); }
inline static clang::AnnotateAttr* IsAccess(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "access"); }
inline static clang::AnnotateAttr* IsInterpolation(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "interpolation"); }
inline static clang::AnnotateAttr* IsGroupShared(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "groupshared"); }
inline static clang::AnnotateAttr* IsStage(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "stage"); }
inline static clang::AnnotateAttr* IsStageInout(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "stage_inout"); }
inline static clang::AnnotateAttr* IsResourceBind(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "binding"); }
inline static clang::AnnotateAttr* IsPushConstant(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "push_constant"); }
inline static clang::AnnotateAttr* IsGloballyCoherent(const clang::Decl* decl) { return ExistShaderAttrWithName(decl, "globallycoherent"); }

const bool LanguageRule_UseAssignForImplicitCopyOrMove(const clang::Decl* x);
const bool LanguageRule_UseMethodForOperatorOverload(const clang::Decl* decl, std::string* pReplaceName);
bool LanguageRule_BanDoubleFieldsAndVariables(const clang::Decl* decl, const clang::QualType& qt);
bool LanguageRule_UseFunctionInsteadOfMethod(const clang::CXXMethodDecl* Method);
bool IsShaderReserveWorld(const std::string& string);

template <typename T>
inline static T GetArgumentAt(const clang::AnnotateAttr* attr, size_t index)
{
    auto args = attr->args_begin() + index;
    if constexpr (std::is_same_v<T, clang::StringRef>)
    {
        auto arg = llvm::dyn_cast<clang::StringLiteral>((*args)->IgnoreParenCasts());
        return arg->getString();
    }
    else if constexpr (std::is_integral_v<T>)
    {
        auto arg = llvm::dyn_cast<clang::IntegerLiteral>((*args)->IgnoreParenCasts());
        return arg->getValue().getLimitedValue();
    }
    else
    {
        static_assert(std::is_same_v<T, std::nullptr_t>, "Unsupported type for GetArgumentAt");
    }
}

} // namespace skr::CppSL