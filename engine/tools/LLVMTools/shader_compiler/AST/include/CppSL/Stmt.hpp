#pragma once
#include <vector>
#include <span>
#include <string>
#include "Enums.hpp"

namespace skr::CppSL {

using Size = uint32_t;
using String = std::wstring;
using Name = String;

struct AST;
struct Attr;
struct Decl;
struct DeclRefExpr;
struct CaseStmt;

struct Stmt
{
public:
    inline std::span<const Stmt* const> children() const { return _children; }
    inline const Stmt* parent() const { return _parent; }

    std::span<Attr* const> attrs() const { return _attrs; }
    void add_attr(Attr* attr);
    void add_attrs(std::span<Attr* const> attrs) { for (auto attr : attrs) add_attr(attr); }

    String dump() const;

protected:
    friend struct AST;
    Stmt(AST& ast);
    virtual ~Stmt() = default;

    void add_child(const Stmt* child);
    const AST* _ast = nullptr;
    const Stmt* _parent = nullptr;
    std::vector<const Stmt*> _children;
    std::vector<Attr*> _attrs;
};

struct DeclStmt : Stmt
{
public:
    DeclRefExpr* ref() const;
    const Decl* decl() const { return _decl; }

protected:
    friend struct AST;
    DeclStmt(AST& ast, Decl* decl);
    // TODO: DeclGroup
    Decl* _decl = nullptr;
};

struct DeclGroupStmt : Stmt
{
protected:
    friend struct AST;
    DeclGroupStmt(AST& ast, std::span<DeclStmt* const> children);
};

struct CompoundStmt final : Stmt
{
public:
    void add_statement(Stmt* statement);
    template <typename... Stmts>
    void add_statements(Stmts... stmts) { (add_statement(stmts), ...); }
    
protected:
    friend struct AST;
    CompoundStmt(AST& ast, std::span<Stmt* const> statements);
};

struct IfStmt final : Stmt
{
public:
    const Stmt* cond() const { return _cond; }
    const CompoundStmt* then_body() const { return _then_body; }
    const CompoundStmt* else_body() const { return _else_body; }

private:
    friend struct AST;
    IfStmt(AST& ast, Stmt* cond, CompoundStmt* then_body, CompoundStmt* else_body);
    Stmt* _cond = nullptr;
    CompoundStmt* _then_body = nullptr;
    CompoundStmt* _else_body = nullptr;
};

struct ForStmt final : Stmt
{
public:
    const Stmt* init() const { return _init; }
    const Stmt* cond() const { return _cond; }
    const Stmt* inc() const { return _inc; }
    const CompoundStmt* body() const { return _body; }

private:
    friend struct AST;
    ForStmt(AST& ast, Stmt* init, Stmt* cond, Stmt* inc, CompoundStmt* body);
    Stmt* _init = nullptr;
    Stmt* _cond = nullptr;
    Stmt* _inc = nullptr;
    CompoundStmt* _body = nullptr;
};

struct WhileStmt final : Stmt
{
public:
    const Stmt* cond() const { return _cond; }
    const CompoundStmt* body() const { return _body; }

private:
    friend struct AST;
    WhileStmt(AST& ast, Stmt* cond, CompoundStmt* body);
    Stmt* _cond = nullptr;
    CompoundStmt* _body = nullptr;
};

struct BreakStmt final : Stmt
{
private:
    friend struct AST;
    BreakStmt(AST& ast);
};

struct ContinueStmt final : Stmt
{
private:
    friend struct AST;
    ContinueStmt(AST& ast);
};

struct CommentStmt final : Stmt
{
public:
    const String& text() const { return _text; }

private:
    friend struct AST;
    CommentStmt(AST& ast, const String& text);
    String _text;
};

struct DefaultStmt final : Stmt
{
private:
    friend struct AST;
    DefaultStmt(AST& ast, CompoundStmt* body);
};

struct SwitchStmt final : Stmt
{
public:
    const Stmt* cond() const { return _cond; }
    const CompoundStmt* body() const { return _body; }

private:
    friend struct AST;
    SwitchStmt(AST& ast, Stmt* cond, CompoundStmt* body);
    Stmt* _cond = nullptr;
    CompoundStmt* _body = nullptr;
};

struct CaseStmt final : Stmt
{
public:
    const Stmt* cond() const { return _cond; }
    const Stmt* body() const { return _body; }

private:
    friend struct AST;
    CaseStmt(AST& ast, Stmt* cond, Stmt* body);
    Stmt* _cond = nullptr;
    Stmt* _body = nullptr;
};

struct ReturnStmt final : Stmt
{
public:
    const Stmt* value() const { return _value; }
protected:
    friend struct AST;
    ReturnStmt(AST& ast, Stmt* value);
    Stmt* _value = nullptr;
};

struct ValueStmt : public Stmt
{
    virtual ~ValueStmt() = default;
protected:
    ValueStmt(AST& ast);
};

} // namespace skr::CppSL