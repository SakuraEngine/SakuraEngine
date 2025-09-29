#pragma once
#include <variant>
#include "Enums.hpp"
#include "Constant.hpp"
#include "Decl.hpp"
#include "Stmt.hpp"

namespace skr::CppSL {

struct BinaryExpr;
struct ParameterExpr;
struct ConstantExpr;
struct InitListExpr;
using FloatSemantics = String;

struct Expr : public ValueStmt
{
public:
    virtual ~Expr() = default;
    const TypeDecl* type() const { return _type; }

protected:
    Expr(AST& ast, const TypeDecl* type);
    const TypeDecl* _type = nullptr;
};

struct AccessExpr : public Expr
{
private:
    friend struct AST;
    AccessExpr(AST& ast, Expr* base, Expr* index);
};

struct CastExpr : public Expr
{
public:
    Expr* expr() const { return _expr; }

protected:
    CastExpr(AST& ast, const TypeDecl* type, Expr* expr);
    Expr* _expr = nullptr;
};

struct BitwiseCastExpr : public CastExpr
{
private:
    friend struct AST;
    BitwiseCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct BinaryExpr : public Expr
{
public:
    const Expr* left() const { return _left; }
    const Expr* right() const { return _right; }
    const BinaryOp op() const { return _op; }

private:
    friend struct AST;
    BinaryExpr(AST& ast, Expr* left, Expr* right, BinaryOp op);
    Expr* _left = nullptr;
    Expr* _right = nullptr;
    const BinaryOp _op = BinaryOp::ADD;
};

struct CallExpr : public Expr
{
public:
    const DeclRefExpr* callee() const { return _callee; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    CallExpr(AST& ast, DeclRefExpr* callee, std::span<Expr*> args);
    const DeclRefExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct ConditionalExpr : public Expr
{
public:
    const Expr* cond() const { return _cond; }
    const Expr* then_expr() const { return _then; }
    const Expr* else_expr() const { return _else; }

private:
    friend struct AST;
    ConditionalExpr(AST& ast, Expr* cond, Expr* _then, Expr* _else);
    Expr* _cond = nullptr;
    Expr* _then = nullptr;
    Expr* _else = nullptr;
};

struct ConstantExpr : public Expr
{
public:
    const std::variant<IntValue, FloatValue> value;

private:
    friend struct AST;
    ConstantExpr(AST& ast, const IntValue& v);
    ConstantExpr(AST& ast, const FloatValue& v);
};

struct ConstructExpr : public Expr
{
public:
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    ConstructExpr(AST& ast, const TypeDecl* type, std::span<Expr*> args);
    std::vector<Expr*> _args;
};

struct DeclRefExpr : public Expr
{
public:
    const Decl* decl() const { return _decl; }

private:
    friend struct AST;
    DeclRefExpr(AST& ast, const Decl& decl);
    const Decl* _decl = nullptr;
};

struct ImplicitCastExpr : public CastExpr
{
private:
    friend struct AST;
    ImplicitCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct InitListExpr : public Expr
{
private:
    friend struct AST;
    InitListExpr(AST& ast, std::span<Expr*> exprs);
    std::vector<Expr*> _exprs;
};

struct MemberExpr : public Expr
{
public:
    const Expr* owner() const { return _owner; }
    const Decl* member_decl() const { return _member_decl; }

protected:
    MemberExpr(AST& ast, const Expr* owner, const Decl* field, const TypeDecl* type);
    const Expr* _owner = nullptr;
    const Decl* _member_decl = nullptr;
};

struct FieldExpr : public MemberExpr
{
public:
    const FieldDecl* field_decl() const;

private:
    friend struct AST;
    FieldExpr(AST& ast, const Expr* owner, const FieldDecl* field);
};

struct MethodExpr : public MemberExpr
{
public:
    const MethodDecl* method_decl() const;
    
private:
    friend struct AST;
    MethodExpr(AST& ast, const Expr* owner, const FunctionDecl* method);
};

struct MethodCallExpr : public Expr
{
public:
    const MemberExpr* callee() const { return _callee; }
    std::span<Expr* const> args() const { return _args; }

private:
    friend struct AST;
    MethodCallExpr(AST& ast, const MemberExpr* callee, std::span<Expr*> args);
    const MemberExpr* _callee = nullptr;
    std::vector<Expr*> _args;
};

struct SwizzleExpr : public Expr
{
public:
    const Expr* expr() const { return _expr; }
    std::span<const uint64_t> seq() const { return std::span<const uint64_t>(_seq, _comps); }

private:
    friend struct AST;
    SwizzleExpr(AST& ast, Expr* expr, const TypeDecl* result_type, uint64_t comps, const uint64_t* seq);
    uint64_t _seq[4];
    uint64_t _comps = 0u;
    Expr* _expr = nullptr;
};

struct StaticCastExpr : public CastExpr
{
private:
    friend struct AST;
    StaticCastExpr(AST& ast, const TypeDecl* type, Expr* expr);
};

struct ThisExpr final : public Expr
{
private:
    friend struct AST;
    ThisExpr(AST& ast, const TypeDecl* type);
};

struct UnaryExpr : public Expr
{
public:
    const UnaryOp op() const { return _op; }
    const Expr* expr() const { return _expr; }
private:
    friend struct AST;
    UnaryExpr(AST& ast, UnaryOp op, Expr* expr);
    const UnaryOp _op = UnaryOp::PLUS;
    Expr* _expr = nullptr;
};

} // namespace skr::CppSL