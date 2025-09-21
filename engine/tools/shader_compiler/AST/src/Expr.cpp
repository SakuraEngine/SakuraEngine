#include "CppSL/Expr.hpp"
#include "CppSL/CppSLAST.hpp"
#include "CppSL/magic_enum/magic_enum.hpp"

namespace skr::CppSL {

inline static const skr::CppSL::TypeDecl* GetElementType(const TypeDecl* type)
{
    if (auto array_type = dynamic_cast<const ArrayTypeDecl*>(type))
    {
        return array_type->element_type();
    }
    else if (auto vector_type = dynamic_cast<const VectorTypeDecl*>(type))
    {
        return vector_type->element_type();
    }
    else if (auto matrix_type = dynamic_cast<const MatrixTypeDecl*>(type))
    {
        const auto n = matrix_type->columns();
        const auto element = matrix_type->element_type();
        return const_cast<AST&>(type->ast()).VectorType(element, n);
    }
    return nullptr;
}

Expr::Expr(AST& ast, const TypeDecl* type) 
    : ValueStmt(ast), _type(type) 
{

}

BinaryExpr::BinaryExpr(AST& ast, Expr* left, Expr* right, BinaryOp op) 
    : Expr(ast, left->type()), _left(left), _right(right), _op(op) 
{
    add_child(left);
    add_child(right);
}

AccessExpr::AccessExpr(AST& ast, Expr* base, Expr* index)
    : Expr(ast, GetElementType(base->type()))
{
    add_child(base);
    add_child(index);
}

CastExpr::CastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : Expr(ast, type), _expr(expr) 
{
    add_child(expr);
}

BitwiseCastExpr::BitwiseCastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr)
{

}

CallExpr::CallExpr(AST& ast, DeclRefExpr* callee, std::span<Expr*> args) 
    : Expr(ast, dynamic_cast<const FunctionDecl*>(callee->decl())->return_type()), _callee(callee), _args(args.begin(), args.end()) 
{
    add_child(callee);
    for (auto arg : _args)
        add_child(arg);
}

ConditionalExpr::ConditionalExpr(AST& ast, Expr* cond, Expr* _then, Expr* _else) 
    : Expr(ast, _then->type()), _cond(cond), _then(_then), _else(_else) 
{
    add_child(cond);
    add_child(_then);
    add_child(_else);
}

inline static const TypeDecl* GetIntType(const AST& ast, const IntValue& v)
{
    const bool is_signed = v.is_signed();
    const uint32_t bitwidth = v.bitwidth();
    if (bitwidth <= 32)
        return is_signed ? ast.IntType : ast.UIntType;
    else
        return is_signed ? ast.I64Type : ast.U64Type;
}

ConstantExpr::ConstantExpr(AST& ast, const IntValue& v) 
    : Expr(ast, GetIntType(ast, v)), value(v) 
{

}

ConstantExpr::ConstantExpr(AST& ast, const FloatValue& v) 
    : Expr(ast, ast.FloatType), value(v)
{
    
}

ConstructExpr::ConstructExpr(AST& ast, const TypeDecl* type, std::span<Expr*> args)
    : Expr(ast, type), _args(args.begin(), args.end())
{
    for (auto arg : _args)
        add_child(arg);
}

DeclRefExpr::DeclRefExpr(AST& ast, const Decl& decl) 
    : Expr(ast, dynamic_cast<const VarDecl*>(&decl) ? &dynamic_cast<const VarDecl*>(&decl)->type() : nullptr), _decl(&decl) 
{

}

ImplicitCastExpr::ImplicitCastExpr(AST& ast, const TypeDecl* type, Expr* expr) 
    : CastExpr(ast, type, expr) 
{

}

InitListExpr::InitListExpr(AST& ast, std::span<Expr*> exprs) 
    : Expr(ast, nullptr), _exprs(exprs.begin(), exprs.end()) 
{
    for (auto expr : _exprs)
        add_child(expr);
}

MemberExpr::MemberExpr(AST& ast, const Expr* owner, const Decl* field, const TypeDecl* type)
    : Expr(ast, type), _owner(owner), _member_decl(field)
{
    add_child(_owner);
}

FieldExpr::FieldExpr(AST& ast, const Expr* owner, const FieldDecl* field)
    : MemberExpr(ast, owner, field, &field->type())
{

}

const FieldDecl* FieldExpr::field_decl() const
{
    return dynamic_cast<const FieldDecl*>(_member_decl);
}

MethodExpr::MethodExpr(AST& ast, const Expr* owner, const FunctionDecl* method)
    : MemberExpr(ast, owner, method, nullptr)
{

}

const MethodDecl* MethodExpr::method_decl() const
{
    return dynamic_cast<const MethodDecl*>(_member_decl);
}

MethodCallExpr::MethodCallExpr(AST& ast, const MemberExpr* callee, std::span<Expr*> args)
    : Expr(ast, dynamic_cast<const MethodDecl*>(callee->member_decl())->return_type()), _callee(callee), _args(args.begin(), args.end())
{
    add_child(_callee);
    for (auto arg : _args)
        add_child(arg);
}

SwizzleExpr::SwizzleExpr(AST& ast, Expr* expr, const TypeDecl* result_type, uint64_t comps, const uint64_t* seq)
    : Expr(ast, result_type), _expr(expr), _comps(comps)
{
    for (uint32_t i = 0; i < comps; i++)
        _seq[i] = seq[i];
    add_child(_expr);
}

StaticCastExpr::StaticCastExpr(AST& ast, const TypeDecl* type, Expr* expr)
    : CastExpr(ast, type, expr)
{

}

ThisExpr::ThisExpr(AST& ast, const TypeDecl* type)
    : Expr(ast, type)
{
    
}

UnaryExpr::UnaryExpr(AST& ast, UnaryOp op, Expr* expr)
    : Expr(ast, expr->type()), _op(op), _expr(expr)
{
    add_child(expr);
}

} // namespace skr::CppSL