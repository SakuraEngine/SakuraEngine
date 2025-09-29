#include "CppSL/CppSLAST.hpp"
#include "CppSL/magic_enum/magic_enum.hpp"
#include <array>
#include <unordered_set>

namespace skr::CppSL 
{

struct VarConcept : public VarConceptDecl
{
    VarConcept(AST& ast, const Name& name, std::function<bool(EVariableQualifier, const TypeDecl*)> validator)
        : VarConceptDecl(ast, name), validator(std::move(validator))
    {

    }
    bool validate(EVariableQualifier qualifier, const TypeDecl* type) const override
    {
        return validator(qualifier, type);
    }
    std::function<bool(EVariableQualifier, const TypeDecl*)> validator;
};

struct TemplateCallable : public TemplateCallableDecl
{
    TemplateCallable(AST& ast, const Name& name, ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
        : TemplateCallableDecl(ast, name, param_concepts), ret_spec(ret_spec)
    {

    }
    TemplateCallable(AST& ast, TypeDecl* owner, const Name& name, ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
        : TemplateCallableDecl(ast, owner, name, param_concepts), ret_spec(ret_spec)
    {

    }
    const TypeDecl* get_return_type_for(std::span<const TypeDecl* const> arg_types) const
    {
        return ret_spec(arg_types);
    }
    ReturnTypeSpecializer ret_spec;
};

AccessExpr* AST::Access(Expr* base, Expr* index)
{
    auto expr = new AccessExpr(*this, base, index);
    emplace_stmt(expr);
    return expr;
}

BinaryExpr* AST::Binary(BinaryOp op, Expr* left, Expr* right)
{
    auto expr = new BinaryExpr(*this, left, right, op);
    emplace_stmt(expr);
    return expr;
}

BitwiseCastExpr* AST::BitwiseCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new BitwiseCastExpr(*this, type, expr);
    emplace_stmt(cast);
    return cast;
}

BreakStmt* AST::Break()
{
    auto stmt = new BreakStmt(*this);
    emplace_stmt(stmt);
    return stmt;
}

CompoundStmt* AST::Block(const std::vector<Stmt*>& statements)
{
    auto exp = new CompoundStmt(*this, statements);
    emplace_stmt(exp);
    return exp;
}

CallExpr* AST::CallFunction(DeclRefExpr* callee, std::span<Expr*> args)
{
    auto expr = new CallExpr(*this, callee, args);
    emplace_stmt(expr);
    return expr;
}

CaseStmt* AST::Case(Expr* cond, CompoundStmt* body)
{
    auto stmt = new CaseStmt(*this, cond, body);
    emplace_stmt(stmt);
    return stmt;
}

MethodCallExpr* AST::CallMethod(MemberExpr* callee, std::span<Expr*> args)
{
    auto expr = new MethodCallExpr(*this, callee, args);
    emplace_stmt(expr);
    return expr;
}

ConditionalExpr* AST::Conditional(Expr* cond, Expr* _then, Expr* _else)
{
    auto expr = new ConditionalExpr(*this, cond, _then, _else);
    emplace_stmt(expr);
    return expr;
}

ConstantExpr* AST::Constant(const IntValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    emplace_stmt(expr);
    return expr;
}

ConstantExpr* AST::Constant(const FloatValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    emplace_stmt(expr);
    return expr;
}

ConstructExpr* AST::Construct(const TypeDecl* type, std::span<Expr*> args)
{
    auto expr = new ConstructExpr(*this, type, args);
    emplace_stmt(expr);
    return expr;
}

ContinueStmt* AST::Continue()
{
    auto stmt = new ContinueStmt(*this);
    emplace_stmt(stmt);
    return stmt;
}

CommentStmt* AST::Comment(const String& text)
{
    auto stmt = new CommentStmt(*this, text);
    emplace_stmt(stmt);
    return stmt;
}

DefaultStmt* AST::Default(CompoundStmt* body)
{
    auto stmt = new DefaultStmt(*this, body);
    emplace_stmt(stmt);
    return stmt;
}

FieldExpr* AST::Field(Expr* base, const FieldDecl* field)
{
    auto expr = new FieldExpr(*this, base, field);
    emplace_stmt(expr);
    return expr;
}

ForStmt* AST::For(Stmt* init, Expr* cond, Stmt* inc, CompoundStmt* body)
{
    auto stmt = new ForStmt(*this, init, cond, inc, body);
    emplace_stmt(stmt);
    return stmt;
}

IfStmt* AST::If(Expr* cond, CompoundStmt* then_body, CompoundStmt* else_body)
{
    auto stmt = new IfStmt(*this, cond, then_body, else_body);
    emplace_stmt(stmt);
    return stmt;
}

InitListExpr* AST::InitList(std::span<Expr*> exprs)
{
    auto expr = new InitListExpr(*this, exprs);
    emplace_stmt(expr);
    return expr;
}

ImplicitCastExpr* AST::ImplicitCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new ImplicitCastExpr(*this, type, expr);
    emplace_stmt(cast);
    return cast;
}

MethodExpr* AST::Method(Expr* base, const MethodDecl* method)
{
    auto expr = new MethodExpr(*this, base, method);
    emplace_stmt(expr);
    return expr;
}

DeclRefExpr* AST::Ref(const Decl* decl)
{
    assert(decl && "DeclRefExpr cannot be created with a null decl");
    auto expr = new DeclRefExpr(*this, *decl);
    emplace_stmt(expr);
    return expr;
}

ReturnStmt* AST::Return(Expr* expr)
{
    auto stmt = new ReturnStmt(*this, expr);
    emplace_stmt(stmt);
    return stmt;
}

StaticCastExpr* AST::StaticCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new StaticCastExpr(*this, type, expr);
    emplace_stmt(cast);
    return cast;
}

SwizzleExpr* AST::Swizzle(Expr* expr, const TypeDecl* type, uint64_t comps, const uint64_t* seq)
{
    auto swizzle = new SwizzleExpr(*this, expr, type, comps, seq);
    emplace_stmt(swizzle);
    return swizzle;
}

SwitchStmt* AST::Switch(Expr* cond, CompoundStmt* body)
{
    auto stmt = new SwitchStmt(*this, cond, body);
    emplace_stmt(stmt);
    return stmt;
}

ThisExpr* AST::This(const TypeDecl* type)
{
    auto expr = new ThisExpr(*this, type);
    emplace_stmt(expr);
    return expr;
}

UnaryExpr* AST::Unary(UnaryOp op, Expr* expr)
{
    auto unary = new UnaryExpr(*this, op, expr);
    emplace_stmt(unary);
    return unary;
}

DeclStmt* AST::Variable(EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer) 
{  
    ReservedWordsCheck(name);
    assert(qualifier != EVariableQualifier::Inout && "Inout qualifier is not allowed for variable declarations");

    auto decl = new VarDecl(*this, qualifier, type, name, initializer);
    emplace_decl(decl);

    auto stmt = new DeclStmt(*this, decl);
    if (initializer)
        stmt->add_child(initializer);
    emplace_stmt(stmt);

    return stmt;
}

DeclGroupStmt* AST::DeclGroup(std::span<DeclStmt* const> children)
{
    auto group = new DeclGroupStmt(*this, children);
    emplace_stmt(group);
    return group;
}

WhileStmt* AST::While(Expr* cond, CompoundStmt* body)
{
    auto stmt = new WhileStmt(*this, cond, body);
    emplace_stmt(stmt);
    return stmt;
}

TypeDecl* AST::DeclareStructure(const Name& name, std::span<FieldDecl*> fields)
{
    ReservedWordsCheck(name);
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;

    auto new_type = new StructureTypeDecl(*this, name, fields, false);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    return new_type;
}

const TypeDecl* AST::DeclareBuiltinType(const Name& name, uint32_t size, uint32_t alignment, std::vector<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;

    auto new_type = new TypeDecl(*this, name, size, alignment, fields, true);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    return new_type;
}

const ScalarTypeDecl* AST::DeclareScalarType(const Name& name, uint32_t size, uint32_t alignment)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;

    auto new_type = new ScalarTypeDecl(*this, name, size, alignment);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    return new_type;
}

const VectorTypeDecl* AST::DeclareVectorType(const TypeDecl* element, uint32_t count, uint32_t alignment)
{
    const auto key = std::make_pair(element, count);
    auto found = _vecs.find(key);
    if (found != _vecs.end())
        return found->second;

    auto new_type = new VectorTypeDecl(*this, element, count, alignment);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _vecs.insert({key, new_type});
    return new_type;
}

const VectorTypeDecl* AST::VectorType(const TypeDecl* element, uint32_t count)
{
    const auto key = std::make_pair(element, count);
    auto found = _vecs.find(key);
    if (found != _vecs.end())
        return found->second;
    ReportFatalError(L"Vector type with element {} and count {} does not exist!", element->name(), count);
    return nullptr;
}

const MatrixTypeDecl* AST::DeclareMatrixType(const TypeDecl* element, uint32_t x, uint32_t y, uint32_t alignment)
{
    const auto key = std::make_pair(element, std::array<uint32_t, 2>{x, y});
    auto found = _matrices.find(key);
    if (found != _matrices.end())
        return found->second;

    auto new_type = new MatrixTypeDecl(*this, element, x, y, alignment);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _matrices.insert({key, new_type});
    return new_type;
}

const MatrixTypeDecl* AST::MatrixType(const TypeDecl* element, uint32_t n)
{
    const auto key = std::make_pair(element, std::array<uint32_t, 2>{n, n});
    auto found = _matrices.find(key);
    if (found != _matrices.end())
        return found->second;
    ReportFatalError(L"Matrix type with element {} and size {}x{} does not exist!", element->name(), n, n);
    return nullptr;
}

const ArrayTypeDecl* AST::ArrayType(const TypeDecl* element, uint32_t count, ArrayFlags flags)
{
    auto found = _arrs.find({element, count});
    if (found != _arrs.end())
        return found->second;

    auto new_type = new ArrayTypeDecl(*this, element, count, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _arrs.insert({{element, count}, new_type});
    return new_type;
}

GlobalVarDecl* AST::DeclareGlobalConstant(const TypeDecl* type, const Name& name, Expr* initializer)
{
    // TODO: CHECK THIS IS NOT RESOURCE TYPE
    if (type == nullptr) 
        ReportFatalError(L"GlobalConstant {}: Type cannot be null for global constant declaration", name);
    
    ReservedWordsCheck(name);
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::Const, type, name, initializer);
    emplace_decl(decl);
    _globals.emplace_back(decl);
    return decl;
}

GlobalVarDecl* AST::DeclareGroupShared(const TypeDecl* type, const Name& name, Expr* initializer)
{
    // TODO: CHECK THIS IS NOT RESOURCE TYPE
    if (type == nullptr) 
        ReportFatalError(L"GlobalConstant {}: Type cannot be null for global constant declaration", name);
    
    ReservedWordsCheck(name);
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::GroupShared, type, name, initializer);
    emplace_decl(decl);
    _globals.emplace_back(decl);
    return decl;
}

GlobalVarDecl* AST::DeclareGlobalResource(const TypeDecl* type, const Name& name)
{
    // TODO: CHECK THIS IS RESOURCE TYPE
    if (type == nullptr) 
        ReportFatalError(L"GlobalResource {}: Type cannot be null for global resource declaration", name);

    ReservedWordsCheck(name);
    auto decl = new GlobalVarDecl(*this, EVariableQualifier::None, type, name, nullptr);
    emplace_decl(decl);
    _globals.emplace_back(decl);
    return decl;
}

FieldDecl* AST::DeclareField(const Name& name, const TypeDecl* type)
{
    if (type == nullptr) 
        ReportFatalError(L"Field {}: Type cannot be null for parameter declaration", name);
    
    ReservedWordsCheck(name);
    auto decl = new FieldDecl(*this, name, type);
    emplace_decl(decl);
    return decl;
}

MethodDecl* AST::DeclareMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new MethodDecl(*this, owner, name, return_type, params, body);
    emplace_decl(decl);
    _methods.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

ConstructorDecl* AST::DeclareConstructor(TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new ConstructorDecl(*this, owner, name, params, body);
    emplace_decl(decl);
    _ctors.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::DeclareFunction(const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    emplace_decl(decl);
    _funcs.emplace_back(decl);
    return decl;
}

NamespaceDecl* AST::DeclareNamespace(const Name& name, NamespaceDecl* parent)
{
    ReservedWordsCheck(name);
    auto decl = new NamespaceDecl(*this, name, parent);
    emplace_decl(decl);
    _namespaces.emplace_back(decl);
    if (parent)
    {
        parent->add_nested(decl);
    }
    return decl;
}

ParamVarDecl* AST::DeclareParam(EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
{
    if (type == nullptr) ReportFatalError(L"Param {}: Type cannot be null for parameter declaration", name);

    ReservedWordsCheck(name);
    auto decl = new ParamVarDecl(*this, qualifier, type, name);
    emplace_decl(decl);
    return decl;
}

VarConceptDecl* AST::DeclareVarConcept(const Name& name, std::function<bool(EVariableQualifier, const TypeDecl*)> validator)
{
    auto decl = new VarConcept(*this, name, std::move(validator));
    emplace_decl(decl);
    return decl;
}

const AccelTypeDecl* AST::Accel()
{
    if (_accel)
        return _accel;

    _accel = new AccelTypeDecl(*this);
    emplace_decl(_accel);
    return _accel;
}

const RayQueryTypeDecl* AST::RayQuery(RayQueryFlags flags)
{
    auto found = _ray_queries.find(flags);
    if (found != _ray_queries.end())
        return found->second;

    auto new_type = new RayQueryTypeDecl(*this, flags);
    emplace_decl(new_type);
    _ray_queries[flags] = new_type;
    return new_type;
}

ByteBufferTypeDecl* AST::ByteBuffer(BufferFlags flags)
{
    const std::pair<const TypeDecl*, BufferFlags> key = { nullptr, flags };
    auto&& iter = _buffers.find(key);
    if (iter != _buffers.end())
        return dynamic_cast<ByteBufferTypeDecl*>(iter->second);

    auto new_type = new ByteBufferTypeDecl(*this, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _buffers[key] = new_type;
    return new_type;
}

ConstantBufferTypeDecl* AST::ConstantBuffer(const TypeDecl* element)
{
    if (element == nullptr) ReportFatalError(L"ConstantBuffer: Element type cannot be null");

    auto&& iter = _cbuffers.find(element);
    if (iter != _cbuffers.end())
        return dynamic_cast<ConstantBufferTypeDecl*>(iter->second);

    auto new_type = new ConstantBufferTypeDecl(*this, element);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _cbuffers[element] = new_type;
    return new_type;
}

StructuredBufferTypeDecl* AST::StructuredBuffer(const TypeDecl* element, BufferFlags flags)
{
    if (element == nullptr) ReportFatalError(L"StructuredBuffer: Element type cannot be null");

    const std::pair<const TypeDecl*, BufferFlags> key = { element, flags };
    auto&& iter = _buffers.find(key);
    if (iter != _buffers.end())
        return dynamic_cast<StructuredBufferTypeDecl*>(iter->second);

    auto new_type = new StructuredBufferTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _buffers[key] = new_type;
    return new_type;
}

TexelBufferTypeDecl* AST::TexelBuffer(const TypeDecl* element, BufferFlags flags)
{
    if (element == nullptr) ReportFatalError(L"TexelBuffer: Element type cannot be null");

    const std::pair<const TypeDecl*, BufferFlags> key = { element, flags };
    auto&& iter = _buffers.find(key);
    if (iter != _buffers.end())
        return dynamic_cast<TexelBufferTypeDecl*>(iter->second);

    auto new_type = new TexelBufferTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _buffers[key] = new_type;
    return new_type;
}

SamplerDecl* AST::Sampler()
{
    if (_sampler)
        return _sampler;

    _sampler = new SamplerDecl(*this);
    emplace_decl(_sampler);
    return _sampler;
}

Texture1DTypeDecl* AST::Texture1D(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_1ds.find(key);
    if (iter != _texture_1ds.end())
        return dynamic_cast<Texture1DTypeDecl*>(iter->second);

    auto new_type = new Texture1DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_1ds[key] = new_type;
    return new_type;
}

Texture2DTypeDecl* AST::Texture2D(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_2ds.find(key);
    if (iter != _texture_2ds.end())
        return dynamic_cast<Texture2DTypeDecl*>(iter->second);

    auto new_type = new Texture2DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_2ds[key] = new_type;
    return new_type;
}

Texture3DTypeDecl* AST::Texture3D(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_3ds.find(key);
    if (iter != _texture_3ds.end())
        return dynamic_cast<Texture3DTypeDecl*>(iter->second);

    auto new_type = new Texture3DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_3ds[key] = new_type;
    return new_type;
}

TextureCubeTypeDecl* AST::TextureCube(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_cubes.find(key);
    if (iter != _texture_cubes.end())
        return dynamic_cast<TextureCubeTypeDecl*>(iter->second);

    auto new_type = new TextureCubeTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_cubes[key] = new_type;
    return new_type;
}

Texture1DArrayTypeDecl* AST::Texture1DArray(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_1das.find(key);
    if (iter != _texture_1das.end())
        return dynamic_cast<Texture1DArrayTypeDecl*>(iter->second);

    auto new_type = new Texture1DArrayTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_1das[key] = new_type;
    return new_type;
}

Texture2DArrayTypeDecl* AST::Texture2DArray(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_2das.find(key);
    if (iter != _texture_2das.end())
        return dynamic_cast<Texture2DArrayTypeDecl*>(iter->second);

    auto new_type = new Texture2DArrayTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_2das[key] = new_type;
    return new_type;
}

Texture3DArrayTypeDecl* AST::Texture3DArray(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture_3das.find(key);
    if (iter != _texture_3das.end())
        return dynamic_cast<Texture3DArrayTypeDecl*>(iter->second);

    auto new_type = new Texture3DArrayTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    emplace_decl(new_type);
    _texture_3das[key] = new_type;
    return new_type;
}

const TypeDecl* AST::GetType(const Name& name) const
{
    ReservedWordsCheck(name);
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return *found;
    return nullptr;
}

template <typename... Args>
std::vector<FieldDecl*> DeclareFields(AST* ast, const TypeDecl* type, Args&&... args)
{
    std::vector<FieldDecl*> fields;
    (fields.emplace_back(ast->DeclareField(std::forward<Args>(args), type)), ...);
    return fields;
}

// Template function/method declarations
TemplateCallableDecl* AST::DeclareTemplateFunction(const Name& name, const TypeDecl* return_type, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, name, [=](auto params){ return return_type; }, param_concepts);
    emplace_decl(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateFunction(const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, name, ret_spec, param_concepts);
    emplace_decl(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, [=](auto params){ return return_type; }, param_concepts);
    emplace_decl(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, ret_spec, param_concepts);
    emplace_decl(decl);
    return decl;
}

SpecializedFunctionDecl* AST::SpecializeTemplateFunction(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers, const TypeDecl* ret_spec)
{
    // Create new specialization
    auto specialized = (SpecializedFunctionDecl*)template_decl->specialize_for(arg_types, arg_qualifiers, ret_spec);
    emplace_decl(specialized);
    return specialized;
}

const TemplateCallableDecl* AST::FindIntrinsic(const char* name) const
{
    auto it = _intrinsics.find(name);
    if (it != _intrinsics.end())
        return it->second;
    return nullptr;
}

bool AST::IsIntrinsic(const FunctionDecl* function) const
{
    if (auto as_spec = dynamic_cast<const SpecializedFunctionDecl*>(function))
    {
        auto prototype = as_spec->template_decl();
        return _intrinsics_set.contains(prototype);
    }
    return false;
}

bool AST::IsIntrinsic(const FunctionDecl* function, const String& prefix) const
{
    if (IsIntrinsic(function))
        return function->name().starts_with(prefix);
    return false;
}

SpecializedMethodDecl* AST::SpecializeTemplateMethod(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers, const TypeDecl* ret_spec)
{
    auto specialized = (SpecializedMethodDecl*)template_decl->specialize_for(arg_types, arg_qualifiers, ret_spec);
    emplace_decl(specialized);
    return specialized;
}

#define USTR(x) L ## #x
#define INIT_BUILTIN_TYPE(symbol, type, name) symbol##Type(DeclareScalarType(USTR(name), sizeof(type), alignof(type)))

#define INIT_VEC_TYPES(symbol, type, name) \
    symbol##2Type(DeclareVectorType(symbol##Type, 2, alignof(vec<type, 2>))), \
    symbol##3Type(DeclareVectorType(symbol##Type, 3, alignof(vec<type, 3>))), \
    symbol##4Type(DeclareVectorType(symbol##Type, 4, alignof(vec<type, 4>)))

#define INIT_MATRIX_TYPE(symbol, type, name) \
    symbol##2x2Type(DeclareMatrixType(symbol##Type, 2, 2, alignof(matrix<type, 2, 2>))), \
    symbol##2x3Type(DeclareMatrixType(symbol##Type, 2, 3, alignof(matrix<type, 2, 3>))), \
    symbol##2x4Type(DeclareMatrixType(symbol##Type, 2, 4, alignof(matrix<type, 2, 4>))), \
    symbol##3x2Type(DeclareMatrixType(symbol##Type, 3, 2, alignof(matrix<type, 3, 2>))), \
    symbol##3x3Type(DeclareMatrixType(symbol##Type, 3, 3, alignof(matrix<type, 3, 3>))), \
    symbol##3x4Type(DeclareMatrixType(symbol##Type, 3, 4, alignof(matrix<type, 3, 4>))), \
    symbol##4x2Type(DeclareMatrixType(symbol##Type, 3, 2, alignof(matrix<type, 4, 2>))), \
    symbol##4x3Type(DeclareMatrixType(symbol##Type, 3, 3, alignof(matrix<type, 4, 3>))), \
    symbol##4x4Type(DeclareMatrixType(symbol##Type, 4, 4, alignof(matrix<type, 4, 4>))) 

AST::AST() :
    VoidType(DeclareBuiltinType(L"void", 0, 0)),
    
    INIT_BUILTIN_TYPE(Bool, GPUBool, bool),
    INIT_VEC_TYPES(Bool, GPUBool, bool),
    // INIT_MATRIX_TYPE(Bool, GPUBool, bool),

    INIT_BUILTIN_TYPE(Half, float, half),
    INIT_VEC_TYPES(Half, float, half),

    INIT_BUILTIN_TYPE(Float, float, float),
    INIT_VEC_TYPES(Float, float, float),
    INIT_MATRIX_TYPE(Float, float, float),

    INIT_BUILTIN_TYPE(Int, int32_t, int),
    INIT_VEC_TYPES(Int, int32_t, int),
    // INIT_MATRIX_TYPE(Int, int32_t, int),

    INIT_BUILTIN_TYPE(UInt, uint32_t, uint),
    INIT_VEC_TYPES(UInt, uint32_t, uint),
    // INIT_MATRIX_TYPE(UInt, uint32_t, uint),    INIT_BUILTIN_TYPE(Double, double, double),
    INIT_BUILTIN_TYPE(I64, int64_t, int64),
    INIT_BUILTIN_TYPE(U64, uint64_t, uint64)
{
    DoubleType = FloatType; // Shaders normally does not support double, so we use FloatType for DoubleType
    DeclareIntrinsics();
}

AST::~AST()
{
    for (auto stmt : _stmts)
    {
        delete stmt;
    }
    _stmts.clear();

    for (auto decl : _decls)
    {
        delete decl;
    }
    _decls.clear();

    for (auto attr : _attrs)
    {
        delete attr;
    }
    _attrs.clear();
}

void AST::DeclareIntrinsics()
{
    auto Vector2D = DeclareVarConcept(L"Vector2D", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type || type == Int2Type || type == UInt2Type || type == Bool2Type;
        });
    auto Vector3D = DeclareVarConcept(L"Vector3D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float3Type || type == Int3Type || type == UInt3Type || type == Bool3Type;
        });
    auto Vector4D = DeclareVarConcept(L"Vector4D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float4Type || type == Int4Type || type == UInt4Type || type == Bool4Type;
        });

    auto IntScalar = DeclareVarConcept(L"IntScalar", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == IntType || type == UIntType || type == I64Type || type == U64Type;
        });
    auto IntVector = DeclareVarConcept(L"IntVector", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Int2Type || type == Int3Type || type == Int4Type ||
                   type == UInt2Type || type == UInt3Type || type == UInt4Type;
        });
        
    auto FloatScalar = DeclareVarConcept(L"FloatScalar",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == FloatType || type == HalfType;
        });
    auto FloatVector = DeclareVarConcept(L"FloatVector",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type || type == Float3Type || type == Float4Type;
        });
    auto FloatVector2D = DeclareVarConcept(L"FloatVector2D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float2Type;
        });
    auto FloatVector3D = DeclareVarConcept(L"FloatVector3D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float3Type;
        });
    auto FloatVector4D = DeclareVarConcept(L"FloatVector4D",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == Float4Type;
        });

    auto ResourceFamily = DeclareVarConcept(L"ResourceFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return dynamic_cast<const skr::CppSL::BufferTypeDecl*>(type) != nullptr;
        });
    auto ValueFamily = DeclareVarConcept(L"ValueFamily", 
        [this, ResourceFamily](EVariableQualifier qualifier, const TypeDecl* type) {
            return dynamic_cast<const skr::CppSL::ValueTypeDecl*>(type) != nullptr;
        });

    auto IntFamily = DeclareVarConcept(L"IntFamily", 
        [this, IntScalar, IntVector](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntScalar->validate(qualifier, type) || IntVector->validate(qualifier, type);
        });
    auto FloatFamily = DeclareVarConcept(L"FloatFamily", 
        [this, FloatScalar, FloatVector](EVariableQualifier qualifier, const TypeDecl* type) {
            return FloatScalar->validate(qualifier, type) || FloatVector->validate(qualifier, type);
        });
    auto BoolFamily = DeclareVarConcept(L"BoolFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return type == BoolType || type == Bool2Type || type == Bool3Type || type == Bool4Type;
        });

    auto ArthmeticFamily = DeclareVarConcept(L"ArithmeticFamily", 
        [this, IntFamily, FloatFamily](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntFamily->validate(qualifier, type) || FloatFamily->validate(qualifier, type);
        });
    auto ArthmeticVectorFamily = DeclareVarConcept(L"ArithmeticVectorFamily", 
        [this, IntVector, FloatVector](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntVector->validate(qualifier, type) || FloatVector->validate(qualifier, type);
        });

    auto MatrixFamily = DeclareVarConcept(L"MatrixFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::MatrixTypeDecl*>(type) != nullptr);
        });

    auto BufferFamily = DeclareVarConcept(L"BufferFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type)  {
            return (dynamic_cast<const skr::CppSL::BufferTypeDecl*>(type) != nullptr);
        });
    auto ByteBufferFamily = DeclareVarConcept(L"ByteBufferFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type)  {
            return (dynamic_cast<const skr::CppSL::ByteBufferTypeDecl*>(type) != nullptr);
        });
    auto StructuredBufferFamily = DeclareVarConcept(L"StructuredBufferFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::StructuredBufferTypeDecl*>(type) != nullptr);
        });
    auto IntBufferFamily = DeclareVarConcept(L"IntBufferFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::StructuredBufferTypeDecl*>(type);
            return (t != nullptr) && ((&t->element_type() == IntType) || (&t->element_type() == UIntType) || 
                                      (&t->element_type() == I64Type) || (&t->element_type() == U64Type));
        });
    auto IntSharedArrayFamily = DeclareVarConcept(L"IntSharedArrayFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::ArrayTypeDecl*>(type);
            return (t != nullptr) && (has_flag(t->flags(), ArrayFlags::Shared)) && (
                    (t->element_type() == IntType) || (t->element_type() == UIntType) || 
                    (t->element_type() == I64Type) || (t->element_type() == U64Type));
        });
    auto AtomicOperableFamily = DeclareVarConcept(L"AtomicOperableFamily",
        [this, IntBufferFamily, IntSharedArrayFamily](EVariableQualifier qualifier, const TypeDecl* type) {
            return IntBufferFamily->validate(qualifier, type) || IntSharedArrayFamily->validate(qualifier, type);
        });

    auto SamplerFamily = DeclareVarConcept(L"SamplerFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::SamplerDecl*>(type) != nullptr);
        });
    auto TextureFamily = DeclareVarConcept(L"TextureFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::TextureTypeDecl*>(type) != nullptr);
        });
    auto Texture2DFamily = DeclareVarConcept(L"Texture2DFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::Texture2DTypeDecl*>(type) != nullptr);
        });
    auto FloatTexture2DFamily = DeclareVarConcept(L"FloatTextureFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::Texture2DTypeDecl*>(type);
            return (t != nullptr) && (&t->element_type() == FloatType);
        });
    auto FloatTexture3DFamily = DeclareVarConcept(L"FloatTexture3DFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::Texture3DTypeDecl*>(type);
            return (t != nullptr) && (&t->element_type() == FloatType);
        });

    auto ReturnFirstArgType = [=](auto pts){ return pts[0]; };
#define ReturnVecWithSameDim(T) \
    auto Return##T##VecWithSameDim = [this](auto pts) {\
        const TypeDecl* inputType = pts[0];\
        if (auto asVectorType = dynamic_cast<const VectorTypeDecl*>(pts[0]))\
        {\
            if (asVectorType->count() == 2)\
                return T##2Type;\
            else if (asVectorType->count() == 3)\
                return T##3Type;\
            else if (asVectorType->count() == 4)\
                return T##4Type;\
        }\
        if (auto AsMatrixType = dynamic_cast<const MatrixTypeDecl*>(pts[0]))\
        {\
            if (AsMatrixType->columns() == 2)\
                return T##2x2Type;\
            if (AsMatrixType->columns() == 3)\
                return T##3x3Type;\
            if (AsMatrixType->columns() == 4)\
                return T##4x4Type;\
        }\
        return IntType;\
    }
    ReturnVecWithSameDim(Bool);
    ReturnVecWithSameDim(Float);
    ReturnVecWithSameDim(Int);
    ReturnVecWithSameDim(UInt);

    std::array<VarConceptDecl*, 1> OneValue = { ValueFamily };
    std::array<VarConceptDecl*, 1> OneArithmetic = { ArthmeticFamily };
    _intrinsics["ABS"] = DeclareTemplateFunction(L"abs", ReturnFirstArgType, OneArithmetic);
    // bit_cast<T> will be override when is called
    _intrinsics["ASFLOAT"] = DeclareTemplateFunction(L"asfloat", ReturnFloatVecWithSameDim, OneArithmetic);
    _intrinsics["ASINT"] = DeclareTemplateFunction(L"asint", ReturnIntVecWithSameDim, OneArithmetic);
    _intrinsics["ASUINT"] = DeclareTemplateFunction(L"asuint", ReturnUIntVecWithSameDim, OneArithmetic);

    std::array<VarConceptDecl*, 2> TwoArithmetic = { ArthmeticFamily, ArthmeticFamily };
    _intrinsics["MIN"] = DeclareTemplateFunction(L"min", ReturnFirstArgType, TwoArithmetic);
    _intrinsics["MAX"] = DeclareTemplateFunction(L"max", ReturnFirstArgType, TwoArithmetic);
    
    std::array<VarConceptDecl*, 3> ThreeArithmetic = { ArthmeticFamily, ArthmeticFamily, ArthmeticFamily };
    _intrinsics["CLAMP"] = DeclareTemplateFunction(L"clamp", ReturnFirstArgType, ThreeArithmetic);
    _intrinsics["LERP"] = DeclareTemplateFunction(L"lerp", ReturnFirstArgType, ThreeArithmetic);

    std::array<VarConceptDecl*, 1> OneArithmeticVec = { ArthmeticVectorFamily };

    std::array<VarConceptDecl*, 1> OneBoolFamily = { BoolFamily };
    _intrinsics["ALL"] = DeclareTemplateFunction(L"all", BoolType, OneBoolFamily);
    _intrinsics["ANY"] = DeclareTemplateFunction(L"any", BoolType, OneBoolFamily);

    std::array<VarConceptDecl*, 1> OneIntFamily = { IntFamily };
    _intrinsics["CLZ"] = DeclareTemplateFunction(L"clz", ReturnFirstArgType, OneIntFamily);
    _intrinsics["CTZ"] = DeclareTemplateFunction(L"ctz", ReturnFirstArgType, OneIntFamily);
    _intrinsics["POPCOUNT"] = DeclareTemplateFunction(L"popcount", ReturnFirstArgType, OneIntFamily);
    _intrinsics["REVERSEBITS"] = DeclareTemplateFunction(L"reversebits", ReturnFirstArgType, OneIntFamily);
    _intrinsics["f16tof32"] = DeclareTemplateFunction(L"f16tof32", ReturnFloatVecWithSameDim, OneIntFamily);

    std::array<VarConceptDecl*, 1> OneFloatFamily = { FloatFamily };
    _intrinsics["SIN"] = DeclareTemplateFunction(L"sin", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["SINH"] = DeclareTemplateFunction(L"sinh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["COS"] = DeclareTemplateFunction(L"cos", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["COSH"] = DeclareTemplateFunction(L"cosh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ATAN"] = DeclareTemplateFunction(L"atan", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ATANH"] = DeclareTemplateFunction(L"atanh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["TAN"] = DeclareTemplateFunction(L"tan", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["TANH"] = DeclareTemplateFunction(L"tanh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ACOS"] = DeclareTemplateFunction(L"acos", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ACOSH"] = DeclareTemplateFunction(L"acosh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ASIN"] = DeclareTemplateFunction(L"asin", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ASINH"] = DeclareTemplateFunction(L"asinh", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["EXP"] = DeclareTemplateFunction(L"exp", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["EXP2"] = DeclareTemplateFunction(L"exp2", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["LOG"] = DeclareTemplateFunction(L"log", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["LOG2"] = DeclareTemplateFunction(L"log2", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["LOG10"] = DeclareTemplateFunction(L"log10", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["EXP10"] = DeclareTemplateFunction(L"exp10", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["SQRT"] = DeclareTemplateFunction(L"sqrt", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["RSQRT"] = DeclareTemplateFunction(L"rsqrt", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["CEIL"] = DeclareTemplateFunction(L"ceil", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["FLOOR"] = DeclareTemplateFunction(L"floor", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["FRACT"] = DeclareTemplateFunction(L"fract", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["TRUNC"] = DeclareTemplateFunction(L"trunc", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ROUND"] = DeclareTemplateFunction(L"round", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["LENGTH"] = DeclareTemplateFunction(L"length", FloatType, OneFloatFamily);
    _intrinsics["SATURATE"] = DeclareTemplateFunction(L"saturate", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["DDX"] = DeclareTemplateFunction(L"ddx", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["DDY"] = DeclareTemplateFunction(L"ddy", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["ISINF"] = DeclareTemplateFunction(L"isinf", ReturnBoolVecWithSameDim, OneFloatFamily);
    _intrinsics["ISNAN"] = DeclareTemplateFunction(L"isnan", ReturnBoolVecWithSameDim, OneFloatFamily);
    _intrinsics["f32tof16"] = DeclareTemplateFunction(L"f32tof16", ReturnUIntVecWithSameDim, OneFloatFamily);

    std::array<VarConceptDecl*, 2> TwoFloatFamily = { FloatFamily, FloatFamily };
    _intrinsics["POW"] = DeclareTemplateFunction(L"pow", ReturnFirstArgType, TwoArithmetic);
    _intrinsics["COPYSIGN"] = DeclareTemplateFunction(L"copysign", ReturnFirstArgType, TwoFloatFamily);
    _intrinsics["ATAN2"] = DeclareTemplateFunction(L"atan2", ReturnFirstArgType, TwoFloatFamily);
    _intrinsics["STEP"] = DeclareTemplateFunction(L"step", ReturnFirstArgType, TwoFloatFamily);
    _intrinsics["FMOD"] = DeclareTemplateFunction(L"fmod", ReturnFirstArgType, TwoFloatFamily);
    _intrinsics["MODF"] = DeclareTemplateFunction(L"modf", ReturnFirstArgType, TwoFloatFamily);

    std::array<VarConceptDecl*, 3> ThreeFloatFamily = { FloatFamily, FloatFamily, FloatFamily };
    _intrinsics["FMA"] = DeclareTemplateFunction(L"fma", ReturnFirstArgType, ThreeFloatFamily);
    _intrinsics["SMOOTHSTEP"] = DeclareTemplateFunction(L"smoothstep", ReturnFirstArgType, ThreeFloatFamily);

    std::array<VarConceptDecl*, 1> OneFloatVector = { FloatVector };
    _intrinsics["NORMALIZE"] = DeclareTemplateFunction(L"normalize", ReturnFirstArgType, OneFloatVector);
    _intrinsics["LENGTH_SQUARED"] = DeclareTemplateFunction(L"length_squared", FloatType, OneFloatVector);

    std::array<VarConceptDecl*, 2> TwoFloatVec = { FloatVector, FloatVector };
    _intrinsics["DOT"] = DeclareTemplateFunction(L"dot", FloatType, TwoFloatVec);
    _intrinsics["CROSS"] = DeclareTemplateFunction(L"cross", ReturnFirstArgType, TwoFloatVec);

    std::array<VarConceptDecl*, 3> ThreeFloat3 = { FloatVector3D, FloatVector3D, FloatVector3D };
    _intrinsics["FACEFORWARD"] = DeclareTemplateFunction(L"faceforward", Float3Type, ThreeFloat3);
    std::array<VarConceptDecl*, 2> TwoFloat3 = { FloatVector3D, FloatVector3D };
    _intrinsics["REFLECT"] = DeclareTemplateFunction(L"reflect", Float3Type, TwoFloat3);

    std::array<VarConceptDecl*, 1> OneMatrix = { MatrixFamily };
    _intrinsics["TRANSPOSE"] = DeclareTemplateFunction(L"transpose", ReturnFirstArgType, OneMatrix);
    _intrinsics["DETERMINANT"] = DeclareTemplateFunction(L"determinant", ReturnFirstArgType, OneMatrix);
    _intrinsics["INVERSE"] = DeclareTemplateFunction(L"inverse", ReturnFirstArgType, OneMatrix);

    std::array<VarConceptDecl*, 3> SelectParams = { ValueFamily, ValueFamily, BoolFamily };
    _intrinsics["SELECT"] = DeclareTemplateFunction(L"select", ReturnFirstArgType, SelectParams);

    std::array<VarConceptDecl*, 2> BufferReadParams = { BufferFamily, IntScalar };
    _intrinsics["BUFFER_READ"] = DeclareTemplateFunction(L"buffer_read", 
        [=](auto pts) { 
            return &dynamic_cast<const StructuredBufferTypeDecl*>(pts[0])->element_type(); 
        }, BufferReadParams);

    std::array<VarConceptDecl*, 3> BufferWriteParams = { BufferFamily, IntScalar, ValueFamily };
    _intrinsics["BUFFER_WRITE"] = DeclareTemplateFunction(L"buffer_write", VoidType, BufferWriteParams);

    std::array<VarConceptDecl*, 2> ByteBufferLoadParams = { ByteBufferFamily, IntScalar };
    // Load<T> Type will be specialized when being called
    _intrinsics["BYTE_BUFFER_READ"] = DeclareTemplateFunction(L"byte_buffer_read", VoidType, ByteBufferLoadParams);
    _intrinsics["BYTE_BUFFER_LOAD"] = DeclareTemplateFunction(L"byte_buffer_load", UIntType, ByteBufferLoadParams);
    _intrinsics["BYTE_BUFFER_LOAD2"] = DeclareTemplateFunction(L"byte_buffer_load2", UIntType, ByteBufferLoadParams);
    _intrinsics["BYTE_BUFFER_LOAD3"] = DeclareTemplateFunction(L"byte_buffer_load3", UIntType, ByteBufferLoadParams);
    _intrinsics["BYTE_BUFFER_LOAD4"] = DeclareTemplateFunction(L"byte_buffer_load4", UIntType, ByteBufferLoadParams);

    std::array<VarConceptDecl*, 3> ByteBufferWriteParams = { ByteBufferFamily, IntScalar, ValueFamily };
    _intrinsics["BYTE_BUFFER_WRITE"] = DeclareTemplateFunction(L"byte_buffer_write", VoidType, ByteBufferWriteParams);

    std::array<VarConceptDecl*, 3> ByteBufferStoreParams = { ByteBufferFamily, IntScalar, IntFamily };
    _intrinsics["BYTE_BUFFER_STORE"] = DeclareTemplateFunction(L"byte_buffer_store", VoidType, ByteBufferStoreParams);
    _intrinsics["BYTE_BUFFER_STORE2"] = DeclareTemplateFunction(L"byte_buffer_store2", VoidType, ByteBufferStoreParams);
    _intrinsics["BYTE_BUFFER_STORE3"] = DeclareTemplateFunction(L"byte_buffer_store3", VoidType, ByteBufferStoreParams);
    _intrinsics["BYTE_BUFFER_STORE4"] = DeclareTemplateFunction(L"byte_buffer_store4", VoidType, ByteBufferStoreParams);
    
    auto AtomicReturnSpec = [=](auto pts) {
        if (auto bufferType = dynamic_cast<const StructuredBufferTypeDecl*>(pts[0])) 
            return &bufferType->element_type();
        else
            return pts[0];
        return pts[0];
    };
    std::array<VarConceptDecl*, 3> AtomicParamsTLS = { ValueFamily, ValueFamily, ValueFamily };
    std::array<VarConceptDecl*, 4> CompareStoreParamsTLS = { ValueFamily, ValueFamily, ValueFamily };
    std::array<VarConceptDecl*, 4> CompareExchangeParamsTLS = { ValueFamily, ValueFamily, ValueFamily, ValueFamily };
    _intrinsics["InterlockedExchange"] = DeclareTemplateFunction(L"InterlockedExchange", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedCompareExchange"] = DeclareTemplateFunction(L"InterlockedCompareExchange", VoidType, CompareExchangeParamsTLS);
    _intrinsics["InterlockedCompareStore"] = DeclareTemplateFunction(L"InterlockedCompareStore", VoidType, CompareStoreParamsTLS);
    _intrinsics["InterlockedAdd"] = DeclareTemplateFunction(L"InterlockedAdd", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedAnd"] = DeclareTemplateFunction(L"InterlockedAnd", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedOr"] = DeclareTemplateFunction(L"InterlockedOr", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedXor"] = DeclareTemplateFunction(L"InterlockedXor", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedMin"] = DeclareTemplateFunction(L"InterlockedMin", VoidType, AtomicParamsTLS);
    _intrinsics["InterlockedMax"] = DeclareTemplateFunction(L"InterlockedMax", VoidType, AtomicParamsTLS);

    std::array<VarConceptDecl*, 2> TextureLoadParams = { TextureFamily, IntVector };
    auto TextureLoadRetSpec = [=](auto pts) { 
        return &dynamic_cast<const TextureTypeDecl*>(pts[0])->element_type(); 
    };
    _intrinsics["Texture1D::Load"] = DeclareTemplateFunction(L"CppSLTexture1DLoad", TextureLoadRetSpec, TextureLoadParams);
    _intrinsics["Texture1D::LoadAtMip"] = DeclareTemplateFunction(L"CppSLTexture1DLoadAtMip", TextureLoadRetSpec, TextureLoadParams);
    _intrinsics["Texture2D::Load"] = DeclareTemplateFunction(L"CppSLTexture2DLoad", TextureLoadRetSpec, TextureLoadParams);
    _intrinsics["Texture2D::LoadAtMip"] = DeclareTemplateFunction(L"CppSLTexture2DLoadAtMip", TextureLoadRetSpec, TextureLoadParams);
    _intrinsics["Texture3D::Load"] = DeclareTemplateFunction(L"CppSLTexture3DLoad", TextureLoadRetSpec, TextureLoadParams);
    
    std::array<VarConceptDecl*, 3> TextureLoadWithOffsetParams = { TextureFamily, IntVector, IntVector };
    auto TextureLoadWithOffsetSpec = [=](auto pts) { 
        return &dynamic_cast<const TextureTypeDecl*>(pts[0])->element_type(); 
    };
    _intrinsics["Texture1D::LoadWithOffset"] = DeclareTemplateFunction(L"CppSLTexture1DLoadWithOffset", TextureLoadWithOffsetSpec, TextureLoadWithOffsetParams);
    _intrinsics["Texture2D::LoadWithOffset"] = DeclareTemplateFunction(L"CppSLTexture2DLoadWithOffset", TextureLoadWithOffsetSpec, TextureLoadWithOffsetParams);
    _intrinsics["Texture3D::LoadWithOffset"] = DeclareTemplateFunction(L"CppSLTexture3DLoadWithOffset", TextureLoadWithOffsetSpec, TextureLoadWithOffsetParams);

    std::array<VarConceptDecl*, 3> TextureWriteParams = { TextureFamily, IntVector, Vector4D };
    _intrinsics["Texture1D::Store"] = DeclareTemplateFunction(L"CppSLTexture1DStore", VoidType, TextureWriteParams);
    _intrinsics["Texture2D::Store"] = DeclareTemplateFunction(L"CppSLTexture2DStore", VoidType, TextureWriteParams);
    _intrinsics["Texture3D::Store"] = DeclareTemplateFunction(L"CppSLTexture3DStore", VoidType, TextureWriteParams);

    std::array<VarConceptDecl*, 1> TextureSizeParams = { TextureFamily };
    _intrinsics["Texture1D::Size"] = DeclareTemplateFunction(L"CppSLTexture1DSize", UIntType, TextureSizeParams);
    _intrinsics["Texture2D::Size"] = DeclareTemplateFunction(L"CppSLTexture2DSize", UInt2Type, TextureSizeParams);
    _intrinsics["Texture2DArray::Size"] = DeclareTemplateFunction(L"CppSLTexture2DArraySize", UInt2Type, TextureSizeParams);
    _intrinsics["Texture3D::Size"] = DeclareTemplateFunction(L"CppSLTexture3DSize", UInt3Type, TextureSizeParams);
    _intrinsics["TextureCube::Size"] = DeclareTemplateFunction(L"CppSLTextureCubeSize", UInt3Type, TextureSizeParams);

    std::array<VarConceptDecl*, 3> SampleParams = { TextureFamily, SamplerFamily, FloatFamily };
    auto SampleRetSpec = [this](auto pts) {
        auto pt = &dynamic_cast<const TextureTypeDecl*>(pts[1])->element_type();
        if (pt == FloatType) return Float4Type;
        if (pt == HalfType) return Half4Type;
        if (pt == IntType) return Int4Type;
        if (pt == UIntType) return UInt4Type;
        if (pt == BoolType) return Bool4Type;
        return (const TypeDecl*)nullptr;
    };
    _intrinsics["Texture1D::Sample"] = DeclareTemplateFunction(L"CppSLTexture1DSample", SampleRetSpec, SampleParams);
    _intrinsics["Texture2D::Sample"] = DeclareTemplateFunction(L"CppSLTexture2DSample", SampleRetSpec, SampleParams);
    _intrinsics["Texture3D::Sample"] = DeclareTemplateFunction(L"CppSLTexture3DSample", SampleRetSpec, SampleParams);
    _intrinsics["TextureCube::Sample"] = DeclareTemplateFunction(L"CppSLTextureCubeSample", SampleRetSpec, SampleParams);

    std::array<VarConceptDecl*, 4> GatherParams = { TextureFamily, SamplerFamily, FloatVector, IntVector };
    auto GatherReturn = [this](auto pts) {
        auto pt = &dynamic_cast<const TextureTypeDecl*>(pts[1])->element_type();
        if (pt == FloatType) return Float4Type;
        if (pt == HalfType) return Half4Type;
        if (pt == IntType) return Int4Type;
        if (pt == UIntType) return UInt4Type;
        if (pt == BoolType) return Bool4Type;
        return (const TypeDecl*)nullptr;
    };
    _intrinsics["Texture2D::Gather"] = DeclareTemplateFunction(L"CppSLTexture2DGather", GatherReturn, GatherParams);
    _intrinsics["Texture2D::GatherRed"] = DeclareTemplateFunction(L"CppSLTexture2DGatherRed", GatherReturn, GatherParams);
    _intrinsics["Texture2D::GatherGreen"] = DeclareTemplateFunction(L"CppSLTexture2DGatherGreen", GatherReturn, GatherParams);
    _intrinsics["Texture2D::GatherBlue"] = DeclareTemplateFunction(L"CppSLTexture2DGatherBlue", GatherReturn, GatherParams);
    _intrinsics["Texture2D::GatherAlpha"] = DeclareTemplateFunction(L"CppSLTexture2DGatherAlpha", GatherReturn, GatherParams);

    std::array<VarConceptDecl*, 5> GatherCmpParams = { TextureFamily, SamplerFamily, FloatVector, FloatScalar, IntVector };
    _intrinsics["Texture2D::GatherCmp"] = DeclareTemplateFunction(L"CppSLTexture2DGatherCmp", GatherReturn, GatherCmpParams);
    _intrinsics["Texture2D::GatherCmpRed"] = DeclareTemplateFunction(L"CppSLTexture2DGatherCmpRed", GatherReturn, GatherCmpParams);
    _intrinsics["Texture2D::GatherCmpGren"] = DeclareTemplateFunction(L"CppSLTexture2DGatherCmpGreen", GatherReturn, GatherCmpParams);
    _intrinsics["Texture2D::GatherCmpBlue"] = DeclareTemplateFunction(L"CppSLTexture2DGatherCmpBlue", GatherReturn, GatherCmpParams);
    _intrinsics["Texture2D::GatherCmpAlpha"] = DeclareTemplateFunction(L"CppSLTexture2DGatherCmpAlpha", GatherReturn, GatherCmpParams);

    std::array<VarConceptDecl*, 4> SampleLevelParams = { TextureFamily, SamplerFamily, FloatFamily, FloatScalar };
    auto SampleLevelRetSpec = [this](auto pts) {
        auto pt = &dynamic_cast<const TextureTypeDecl*>(pts[1])->element_type();
        if (pt == FloatType) return Float4Type;
        if (pt == HalfType) return Half4Type;
        if (pt == IntType) return Int4Type;
        if (pt == UIntType) return UInt4Type;
        if (pt == BoolType) return Bool4Type;
        return (const TypeDecl*)nullptr;
    };
    _intrinsics["Texture1D::SampleLevel"] = DeclareTemplateFunction(L"CppSLTexture1DSampleLevel", SampleLevelRetSpec, SampleLevelParams);
    _intrinsics["Texture2D::SampleLevel"] = DeclareTemplateFunction(L"CppSLTexture2DSampleLevel", SampleLevelRetSpec, SampleLevelParams);
    _intrinsics["Texture3D::SampleLevel"] = DeclareTemplateFunction(L"CppSLTexture3DSampleLevel", SampleLevelRetSpec, SampleLevelParams);
    _intrinsics["TextureCube::SampleLevel"] = DeclareTemplateFunction(L"CppSLTextureCubeSampleLevel", SampleLevelRetSpec, SampleLevelParams);

    // TraceRayInline intrinsic
    auto AccelFamily = DeclareVarConcept(L"AccelFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::AccelTypeDecl*>(type) != nullptr);
        });

    // RayPipeline
    _intrinsics["RayPipeline::DispatchRaysIndex"] = DeclareTemplateFunction(L"DispatchRaysIndex", UInt3Type, {});
    std::array<VarConceptDecl*, 8> TraceRayParams = { AccelFamily, IntScalar, IntScalar, IntScalar, IntScalar, IntScalar, ValueFamily, ValueFamily };
    _intrinsics["RayPipeline::TraceRay"] = DeclareTemplateFunction(L"ray_pipe_trace_ray", VoidType, TraceRayParams);
    _intrinsics["RayPipeline::InstanceIndex"] = DeclareTemplateFunction(L"InstanceIndex", UIntType, {});
    _intrinsics["RayPipeline::GeometryIndex"] = DeclareTemplateFunction(L"GeometryIndex", UIntType, {});
    _intrinsics["RayPipeline::PrimitiveIndex"] = DeclareTemplateFunction(L"PrimitiveIndex", UIntType, {});
    _intrinsics["RayPipeline::RayTMin"] = DeclareTemplateFunction(L"RayTMin", FloatType, {});
    _intrinsics["RayPipeline::RayTCurrent"] = DeclareTemplateFunction(L"RayTCurrent", FloatType, {});
    _intrinsics["RayPipeline::WorldRayOrigin"] = DeclareTemplateFunction(L"WorldRayOrigin", Float3Type, {});
    _intrinsics["RayPipeline::WorldRayDirection"] = DeclareTemplateFunction(L"WorldRayDirection", Float3Type, {});

    // RayQuery family concept
    auto RayQueryFamily = DeclareVarConcept(L"RayQueryFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::RayQueryTypeDecl*>(type) != nullptr);
        });

    std::array<VarConceptDecl*, 1> RayQueryProceedParams = { RayQueryFamily };
    _intrinsics["RAY_QUERY_PROCEED"] = DeclareTemplateFunction(L"ray_query_proceed", BoolType, RayQueryProceedParams);
    
    _intrinsics["RAY_QUERY_COMMITTED_STATUS"] = DeclareTemplateFunction(L"ray_query_committed_status", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_TRIANGLE_BARYCENTRICS"] = DeclareTemplateFunction(L"ray_query_committed_triangle_bary", Float2Type, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_PRIMIVE_INDEX"] = DeclareTemplateFunction(L"ray_query_committed_primitive_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_GEOMETRY_INDEX"] = DeclareTemplateFunction(L"ray_query_committed_geometry_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_INSTANCE_INDEX"] = DeclareTemplateFunction(L"ray_query_committed_instance_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_PROCEDURAL_DISTANCE"] = DeclareTemplateFunction(L"ray_query_committed_procedual_distance", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_COMMITTED_RAY_T"] = DeclareTemplateFunction(L"ray_query_committed_ray_t", FloatType, RayQueryProceedParams);
    
    _intrinsics["RAY_QUERY_CANDIDATE_STATUS"] = DeclareTemplateFunction(L"ray_query_candidate_status", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_TRIANGLE_BARYCENTRICS"] = DeclareTemplateFunction(L"ray_query_candidate_triangle_bary", Float2Type, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_PRIMIVE_INDEX"] = DeclareTemplateFunction(L"ray_query_candidate_primitive_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_GEOMETRY_INDEX"] = DeclareTemplateFunction(L"ray_query_candidate_geometry_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_INSTANCE_INDEX"] = DeclareTemplateFunction(L"ray_query_candidate_instance_index", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_PROCEDURAL_DISTANCE"] = DeclareTemplateFunction(L"ray_query_candidate_procedual_distance", UIntType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_CANDIDATE_TRIANGLE_RAY_T"] = DeclareTemplateFunction(L"ray_query_candidate_triangle_ray_t", FloatType, RayQueryProceedParams);
    
    _intrinsics["RAY_QUERY_WORLD_RAY_ORIGIN"] = DeclareTemplateFunction(L"ray_query_world_ray_origin", Float3Type, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_WORLD_RAY_DIRECTION"] = DeclareTemplateFunction(L"ray_query_world_ray_direction", Float3Type, RayQueryProceedParams);
    
    _intrinsics["RAY_QUERY_COMMIT_TRIANGLE"] = DeclareTemplateFunction(L"ray_query_commit_triangle", BoolType, RayQueryProceedParams);
    _intrinsics["RAY_QUERY_TERMINATE"] = DeclareTemplateFunction(L"ray_query_terminate", BoolType, RayQueryProceedParams);

    // void trace_ray_inline(RaytracingAccelerationStructure& AS, uint32 mask, Trait<Ray> ray);
    std::array<VarConceptDecl*, 4> TraceRayInlineParams = { RayQueryFamily, AccelFamily, IntScalar, ValueFamily };
    _intrinsics["RAY_QUERY_TRACE_RAY_INLINE"] = DeclareTemplateFunction(L"ray_query_trace_ray_inline", VoidType, TraceRayInlineParams);

    // commit_procedural with float parameter
    std::array<VarConceptDecl*, 1> CommitProceduralParams = { FloatScalar };
    _intrinsics["RAY_QUERY_COMMIT_PROCEDURAL"] = DeclareTemplateFunction(L"ray_query_commit_procedural", VoidType, CommitProceduralParams);

    // SM 5.1
    _intrinsics["AllMemoryBarrier"] = DeclareTemplateFunction(L"AllMemoryBarrier", VoidType, {});
    _intrinsics["AllMemoryBarrierWithGroupSync"] = DeclareTemplateFunction(L"AllMemoryBarrierWithGroupSync", VoidType, {});
    _intrinsics["GroupMemoryBarrier"] = DeclareTemplateFunction(L"GroupMemoryBarrier", VoidType, {});
    _intrinsics["GroupMemoryBarrierWithGroupSync"] = DeclareTemplateFunction(L"GroupMemoryBarrierWithGroupSync", VoidType, {});
    _intrinsics["DeviceMemoryBarrier"] = DeclareTemplateFunction(L"DeviceMemoryBarrier", VoidType, {});
    _intrinsics["DeviceMemoryBarrierWithGroupSync"] = DeclareTemplateFunction(L"DeviceMemoryBarrierWithGroupSync", VoidType, {});

    // SM 6.1 Wave Intrinstics
    std::array<VarConceptDecl*, 2> ReadLaneAtArgs = { ValueFamily, IntScalar };
    _intrinsics["QuadReadAcrossDiagonal"] = DeclareTemplateFunction(L"QuadReadAcrossDiagonal", ReturnFirstArgType, OneValue);
    _intrinsics["QuadReadLaneAt"] = DeclareTemplateFunction(L"QuadReadLaneAt", ReturnFirstArgType, ReadLaneAtArgs);
    _intrinsics["QuadReadAcrossX"] = DeclareTemplateFunction(L"QuadReadAcrossX", ReturnFirstArgType, OneValue);
    _intrinsics["QuadReadAcrossY"] = DeclareTemplateFunction(L"QuadReadAcrossY", ReturnFirstArgType, OneValue);

    _intrinsics["WaveActiveAllEqual"] = DeclareTemplateFunction(L"WaveActiveAllEqual", ReturnBoolVecWithSameDim, OneArithmeticVec);
    _intrinsics["WaveActiveBitAnd"] = DeclareTemplateFunction(L"WaveActiveBitAnd", ReturnFirstArgType, OneIntFamily);
    _intrinsics["WaveActiveBitOr"] = DeclareTemplateFunction(L"WaveActiveBitOr", ReturnFirstArgType, OneIntFamily);
    _intrinsics["WaveActiveBitXor"] = DeclareTemplateFunction(L"WaveActiveBitXor", ReturnFirstArgType, OneIntFamily);
    _intrinsics["WaveActiveCountBits"] = DeclareTemplateFunction(L"WaveActiveCountBits", UIntType, OneBoolFamily);
    _intrinsics["WaveActiveMax"] = DeclareTemplateFunction(L"WaveActiveMax", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WaveActiveMin"] = DeclareTemplateFunction(L"WaveActiveMin", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WaveActiveProduct"] = DeclareTemplateFunction(L"WaveActiveProduct", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WaveActiveSum"] = DeclareTemplateFunction(L"WaveActiveSum", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WaveActiveAllTrue"] = DeclareTemplateFunction(L"WaveActiveAllTrue", BoolType, OneBoolFamily);
    _intrinsics["WaveActiveAnyTrue"] = DeclareTemplateFunction(L"WaveActiveAnyTrue", BoolType, OneBoolFamily);
    _intrinsics["WaveActiveBallot"] = DeclareTemplateFunction(L"WaveActiveBallot", UInt4Type, OneBoolFamily);
    _intrinsics["WaveGetLaneCount"] = DeclareTemplateFunction(L"WaveGetLaneCount", UIntType, {});
    _intrinsics["WaveGetLaneIndex"] = DeclareTemplateFunction(L"WaveGetLaneIndex", UIntType, {});
    _intrinsics["WaveIsFirstLane"] = DeclareTemplateFunction(L"WaveIsFirstLane", BoolType, {});
    _intrinsics["WavePrefixCountBits"] = DeclareTemplateFunction(L"WavePrefixCountBits", UIntType, OneBoolFamily);
    _intrinsics["WavePrefixProduct"] = DeclareTemplateFunction(L"WavePrefixProduct", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WavePrefixSum"] = DeclareTemplateFunction(L"WavePrefixSum", ReturnFirstArgType, OneArithmetic);
    // WaveReadLaneFirst<T> will be override when is called
    _intrinsics["WaveReadLaneFirst"] = DeclareTemplateFunction(L"WaveReadLaneFirst", ReturnFirstArgType, OneArithmetic);
    _intrinsics["WaveReadLaneAt"] = DeclareTemplateFunction(L"WaveReadLaneAt", VoidType, OneIntFamily);

    _intrinsics["firstbithigh"] = DeclareTemplateFunction(L"firstbithigh", IntType, OneIntFamily);
    for (auto&& [name, intrin] : _intrinsics)
        _intrinsics_set.insert(intrin);
}

void AST::emplace_stmt(Stmt* stmt)
{
    _stmts.emplace_back(stmt);
}

void AST::emplace_decl(Decl* decl)
{
    _decls.emplace_back(decl);
}

void AST::emplace_attr(Attr* attr)
{
    _attrs.emplace_back(attr);
}

SemanticType AST::GetSemanticTypeFromString(const char* str)
{
    if (_semantic_map.empty())
    {
        for (uint32_t i = 0; i < (uint32_t)SemanticType::Count; ++i)
        {
            std::string_view nv = magic_enum::enum_name((SemanticType)i);
            std::string n(nv.data(), nv.size());
            _semantic_map[n] = (SemanticType)i;
        }
    }

    if (_semantic_map.contains(str))
        return _semantic_map[str];
    return SemanticType::Invalid;
}

InterpolationMode AST::GetInterpolationModeFromString(const char* str)
{
    if (_interpolation_map.empty())
    {
        for (uint32_t i = 0; i < (uint32_t)InterpolationMode::Count; ++i)
        {
            std::string_view nv = magic_enum::enum_name((InterpolationMode)i);
            std::string n(nv.data(), nv.size());
            _interpolation_map[n] = (InterpolationMode)i;
        }
    }

    if (_interpolation_map.contains(str))
        return _interpolation_map[str];
    return InterpolationMode::invalid;
}

template <typename... Args>
[[noreturn]] void AST::ReportFatalError(std::wformat_string<Args...> fmt, Args&&... args) const
{
    ReportFatalError(std::format(fmt, std::forward<Args>(args)...));
}

[[noreturn]] void AST::ReportFatalError(const String& message) const
{
    std::wcerr << dump() << std::endl;
    std::wcerr << message << std::endl;
    abort();
}

void AST::ReservedWordsCheck(const Name& name) const
{
    static const std::unordered_set<Name> reserved_names = {
        L"float", L"int", L"uint", L"bool", L"void",
        L"half", L"double", L"int64_t", L"uint64_t"
    };
    if (reserved_names.contains(name))
    {
        ReportFatalError(L"{} is a reserved word, which should not be used!", name);
    }
}

} // namespace skr::CppSL