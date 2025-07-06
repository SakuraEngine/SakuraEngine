#include "CppSL/AST.hpp"
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
    _stmts.emplace_back(expr);
    return expr;
}

BinaryExpr* AST::Binary(BinaryOp op, Expr* left, Expr* right)
{
    auto expr = new BinaryExpr(*this, left, right, op);
    _stmts.emplace_back(expr);
    return expr;
}

BitwiseCastExpr* AST::BitwiseCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new BitwiseCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

BreakStmt* AST::Break()
{
    auto stmt = new BreakStmt(*this);
    _stmts.emplace_back(stmt);
    return stmt;
}

CompoundStmt* AST::Block(const std::vector<Stmt*>& statements)
{
    auto exp = new CompoundStmt(*this, statements);
    _stmts.emplace_back(exp);
    return exp;
}

CallExpr* AST::CallFunction(DeclRefExpr* callee, std::span<Expr*> args)
{
    auto expr = new CallExpr(*this, callee, args);
    _stmts.emplace_back(expr);
    return expr;
}

CaseStmt* AST::Case(Expr* cond, CompoundStmt* body)
{
    auto stmt = new CaseStmt(*this, cond, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

MethodCallExpr* AST::CallMethod(MemberExpr* callee, std::span<Expr*> args)
{
    auto expr = new MethodCallExpr(*this, callee, args);
    _stmts.emplace_back(expr);
    return expr;
}

ConditionalExpr* AST::Conditional(Expr* cond, Expr* _then, Expr* _else)
{
    auto expr = new ConditionalExpr(*this, cond, _then, _else);
    _stmts.emplace_back(expr);
    return expr;
}

ConstantExpr* AST::Constant(const IntValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    _stmts.emplace_back(expr);
    return expr;
}

ConstantExpr* AST::Constant(const FloatValue& v) 
{ 
    auto expr = new ConstantExpr(*this, v); 
    _stmts.emplace_back(expr);
    return expr;
}

ConstructExpr* AST::Construct(const TypeDecl* type, std::span<Expr*> args)
{
    auto expr = new ConstructExpr(*this, type, args);
    _stmts.emplace_back(expr);
    return expr;
}

ContinueStmt* AST::Continue()
{
    auto stmt = new ContinueStmt(*this);
    _stmts.emplace_back(stmt);
    return stmt;
}

CommentStmt* AST::Comment(const String& text)
{
    auto stmt = new CommentStmt(*this, text);
    _stmts.emplace_back(stmt);
    return stmt;
}

DefaultStmt* AST::Default(CompoundStmt* body)
{
    auto stmt = new DefaultStmt(*this, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

FieldExpr* AST::Field(Expr* base, const FieldDecl* field)
{
    auto expr = new FieldExpr(*this, base, field);
    _stmts.emplace_back(expr);
    return expr;
}

ForStmt* AST::For(Stmt* init, Expr* cond, Stmt* inc, CompoundStmt* body)
{
    auto stmt = new ForStmt(*this, init, cond, inc, body);
    _stmts.emplace_back(stmt);
    return stmt;
}

IfStmt* AST::If(Expr* cond, CompoundStmt* then_body, CompoundStmt* else_body)
{
    auto stmt = new IfStmt(*this, cond, then_body, else_body);
    _stmts.emplace_back(stmt);
    return stmt;
}

InitListExpr* AST::InitList(std::span<Expr*> exprs)
{
    auto expr = new InitListExpr(*this, exprs);
    _stmts.emplace_back(expr);
    return expr;
}

ImplicitCastExpr* AST::ImplicitCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new ImplicitCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

MethodExpr* AST::Method(DeclRefExpr* base, const MethodDecl* method)
{
    auto expr = new MethodExpr(*this, base, method);
    _stmts.emplace_back(expr);
    return expr;
}

DeclRefExpr* AST::Ref(const Decl* decl)
{
    assert(decl && "DeclRefExpr cannot be created with a null decl");
    auto expr = new DeclRefExpr(*this, *decl);
    _stmts.emplace_back(expr);
    return expr;
}

ReturnStmt* AST::Return(Expr* expr)
{
    auto stmt = new ReturnStmt(*this, expr);
    _stmts.emplace_back(stmt);
    return stmt;
}

StaticCastExpr* AST::StaticCast(const TypeDecl* type, Expr* expr)
{
    auto cast = new StaticCastExpr(*this, type, expr);
    _stmts.emplace_back(cast);
    return cast;
}

SwizzleExpr* AST::Swizzle(Expr* expr, const TypeDecl* type, uint64_t comps, const uint64_t* seq)
{
    auto swizzle = new SwizzleExpr(*this, expr, type, comps, seq);
    _stmts.emplace_back(swizzle);
    return swizzle;
}

SwitchStmt* AST::Switch(Expr* cond, std::span<CaseStmt*> cases)
{
    auto stmt = new SwitchStmt(*this, cond, cases);
    _stmts.emplace_back(stmt);
    return stmt;
}

ThisExpr* AST::This(const TypeDecl* type)
{
    auto expr = new ThisExpr(*this, type);
    _stmts.emplace_back(expr);
    return expr;
}

UnaryExpr* AST::Unary(UnaryOp op, Expr* expr)
{
    auto unary = new UnaryExpr(*this, op, expr);
    _stmts.emplace_back(unary);
    return unary;
}

DeclStmt* AST::Variable(EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer) 
{  
    ReservedWordsCheck(name);
    assert(qualifier != EVariableQualifier::Inout && "Inout qualifier is not allowed for variable declarations");

    auto decl = new VarDecl(*this, qualifier, type, name, initializer);
    _decls.emplace_back(decl);

    auto stmt = new DeclStmt(*this, decl);
    _stmts.emplace_back(stmt);

    return stmt;
}

DeclGroupStmt* AST::DeclGroup(std::span<DeclStmt* const> children)
{
    auto group = new DeclGroupStmt(*this, children);
    _stmts.emplace_back(group);
    return group;
}

WhileStmt* AST::While(Expr* cond, CompoundStmt* body)
{
    auto stmt = new WhileStmt(*this, cond, body);
    _stmts.emplace_back(stmt);
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
    _decls.emplace_back(new_type);
    return new_type;
}

const TypeDecl* AST::DeclareBuiltinType(const Name& name, uint32_t size, uint32_t alignment, std::vector<FieldDecl*> fields)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;

    auto new_type = new TypeDecl(*this, name, size, alignment, fields, true);
    _types.emplace_back(new_type);
    _decls.emplace_back(new_type);
    return new_type;
}

const ScalarTypeDecl* AST::DeclareScalarType(const Name& name, uint32_t size, uint32_t alignment)
{
    auto found = std::find_if(_types.begin(), _types.end(), [&](auto t){ return t->name() == name; });
    if (found != _types.end())
        return nullptr;

    auto new_type = new ScalarTypeDecl(*this, name, size, alignment);
    _types.emplace_back(new_type);
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(new_type);
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

const MatrixTypeDecl* AST::DeclareMatrixType(const TypeDecl* element, uint32_t n, uint32_t alignment)
{
    const auto key = std::make_pair(element, std::array<uint32_t, 2>{n, n});
    auto found = _matrices.find(key);
    if (found != _matrices.end())
        return found->second;

    auto new_type = new MatrixTypeDecl(*this, element, n, alignment);
    _types.emplace_back(new_type);
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(decl);
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
    _decls.emplace_back(decl);
    _globals.emplace_back(decl);
    return decl;
}

FieldDecl* AST::DeclareField(const Name& name, const TypeDecl* type)
{
    if (type == nullptr) 
        ReportFatalError(L"Field {}: Type cannot be null for parameter declaration", name);
    
    ReservedWordsCheck(name);
    auto decl = new FieldDecl(*this, name, type);
    _decls.emplace_back(decl);
    return decl;
}

MethodDecl* AST::DeclareMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new MethodDecl(*this, owner, name, return_type, params, body);
    _decls.emplace_back(decl);
    _methods.emplace_back(decl);
    return decl;
}

ConstructorDecl* AST::DeclareConstructor(TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new ConstructorDecl(*this, owner, name, params, body);
    _decls.emplace_back(decl);
    _ctors.emplace_back(decl);
    return decl;
}

FunctionDecl* AST::DeclareFunction(const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, CompoundStmt* body)
{
    ReservedWordsCheck(name);
    auto decl = new FunctionDecl(*this, name, return_type, params, body);
    _decls.emplace_back(decl);
    _funcs.emplace_back(decl);
    return decl;
}

ParamVarDecl* AST::DeclareParam(EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
{
    if (type == nullptr) ReportFatalError(L"Param {}: Type cannot be null for parameter declaration", name);

    ReservedWordsCheck(name);
    auto decl = new ParamVarDecl(*this, qualifier, type, name);
    _decls.emplace_back(decl);
    return decl;
}

VarConceptDecl* AST::DeclareVarConcept(const Name& name, std::function<bool(EVariableQualifier, const TypeDecl*)> validator)
{
    auto decl = new VarConcept(*this, name, std::move(validator));
    _decls.emplace_back(decl);
    return decl;
}

const AccelTypeDecl* AST::Accel()
{
    if (_accel)
        return _accel;

    _accel = new AccelTypeDecl(*this);
    _decls.emplace_back(_accel);
    return _accel;
}

const RayQueryTypeDecl* AST::RayQuery(RayQueryFlags flags)
{
    auto found = _ray_queries.find(flags);
    if (found != _ray_queries.end())
        return found->second;

    auto new_type = new RayQueryTypeDecl(*this, flags);
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(new_type);
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
    _decls.emplace_back(new_type);
    _buffers[key] = new_type;
    return new_type;
}

SamplerDecl* AST::Sampler()
{
    if (_sampler)
        return _sampler;

    _sampler = new SamplerDecl(*this);
    _decls.emplace_back(_sampler);
    return _sampler;
}

Texture2DTypeDecl* AST::Texture2D(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture2ds.find(key);
    if (iter != _texture2ds.end())
        return dynamic_cast<Texture2DTypeDecl*>(iter->second);

    auto new_type = new Texture2DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    _decls.emplace_back(new_type);
    _texture2ds[key] = new_type;
    return new_type;
}

Texture3DTypeDecl* AST::Texture3D(const TypeDecl* element, TextureFlags flags)
{
    const std::pair<const TypeDecl*, TextureFlags> key = { element, flags };
    auto&& iter = _texture3ds.find(key);
    if (iter != _texture3ds.end())
        return dynamic_cast<Texture3DTypeDecl*>(iter->second);

    auto new_type = new Texture3DTypeDecl(*this, element, flags);
    _types.emplace_back(new_type);
    _decls.emplace_back(new_type);
    _texture3ds[key] = new_type;
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
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateFunction(const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, name, ret_spec, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, [=](auto params){ return return_type; }, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

TemplateCallableDecl* AST::DeclareTemplateMethod(TypeDecl* owner, const Name& name, TemplateCallableDecl::ReturnTypeSpecializer ret_spec, std::span<const VarConceptDecl* const> param_concepts)
{
    auto decl = new TemplateCallable(*this, owner, name, ret_spec, param_concepts);
    _decls.emplace_back(decl);
    return decl;
}

SpecializedFunctionDecl* AST::SpecializeTemplateFunction(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers)
{
    // Create new specialization
    auto specialized = (SpecializedFunctionDecl*)template_decl->specialize_for(arg_types, arg_qualifiers);
    _decls.emplace_back(specialized);
    return specialized;
}

const TemplateCallableDecl* AST::FindIntrinsic(const char* name) const
{
    auto it = _intrinstics.find(name);
    if (it != _intrinstics.end())
        return it->second;
    return nullptr;
}

SpecializedMethodDecl* AST::SpecializeTemplateMethod(const TemplateCallableDecl* template_decl, std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers)
{
    auto specialized = (SpecializedMethodDecl*)template_decl->specialize_for(arg_types, arg_qualifiers);
    _decls.emplace_back(specialized);
    return specialized;
}

#define USTR(x) L ## #x
#define INIT_BUILTIN_TYPE(symbol, type, name) symbol##Type(DeclareScalarType(USTR(name), sizeof(type), alignof(type)))

#define INIT_VEC_TYPES(symbol, type, name) \
    symbol##2Type(DeclareVectorType(symbol##Type, 2, alignof(vec<type, 2>))), \
    symbol##3Type(DeclareVectorType(symbol##Type, 3, alignof(vec<type, 3>))), \
    symbol##4Type(DeclareVectorType(symbol##Type, 4, alignof(vec<type, 4>)))

#define INIT_MATRIX_TYPE(symbol, type, name) \
    symbol##2x2Type(DeclareMatrixType(symbol##Type, 2, alignof(matrix<type, 2, 2>))), \
    symbol##3x3Type(DeclareMatrixType(symbol##Type, 3, alignof(matrix<type, 3, 3>))), \
    symbol##4x4Type(DeclareMatrixType(symbol##Type, 4, alignof(matrix<type, 4, 4>))) 

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
    DeclareIntrinstics();
}

void AST::DeclareIntrinstics()
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
    auto StructuredBufferFamily = DeclareVarConcept(L"StructuredBufferFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::StructuredBufferTypeDecl*>(type) != nullptr);
        });
    auto IntBufferFamily = DeclareVarConcept(L"IntBufferFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::StructuredBufferTypeDecl*>(type);
            return (t != nullptr) && ((&t->element() == IntType) || (&t->element() == UIntType) || 
                                      (&t->element() == I64Type) || (&t->element() == U64Type));
        });
    auto IntSharedArrayFamily = DeclareVarConcept(L"IntSharedArrayFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::ArrayTypeDecl*>(type);
            return (t != nullptr) && (has_flag(t->flags(), ArrayFlags::Shared)) && (
                    (t->element() == IntType) || (t->element() == UIntType) || 
                    (t->element() == I64Type) || (t->element() == U64Type));
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
            return (t != nullptr) && (&t->element() == FloatType);
        });
    auto FloatTexture3DFamily = DeclareVarConcept(L"FloatTexture3DFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            auto t = dynamic_cast<const skr::CppSL::Texture3DTypeDecl*>(type);
            return (t != nullptr) && (&t->element() == FloatType);
        });

    auto ReturnFirstArgType = [=](auto pts){ return pts[0]; };
    auto ReturnBoolVecWithSameDim = [this](auto pts) {
        const TypeDecl* inputType = pts[0];
        if (inputType == Float2Type || inputType == Int2Type || inputType == UInt2Type) return Bool2Type;
        if (inputType == Float3Type || inputType == Int3Type || inputType == UInt3Type) return Bool3Type;
        if (inputType == Float4Type || inputType == Int4Type || inputType == UInt4Type) return Bool4Type;
        return BoolType; 
    };

    std::array<VarConceptDecl*, 1> OneArithmetic = { ArthmeticFamily };
    _intrinstics["ABS"] = DeclareTemplateFunction(L"abs", ReturnFirstArgType, OneArithmetic);

    std::array<VarConceptDecl*, 2> TwoArithmetic = { ArthmeticFamily, ArthmeticFamily };
    _intrinstics["MIN"] = DeclareTemplateFunction(L"min", ReturnFirstArgType, TwoArithmetic);
    _intrinstics["MAX"] = DeclareTemplateFunction(L"max", ReturnFirstArgType, TwoArithmetic);
    
    std::array<VarConceptDecl*, 3> ThreeArithmetic = { ArthmeticFamily, ArthmeticFamily, ArthmeticFamily };
    _intrinstics["CLAMP"] = DeclareTemplateFunction(L"clamp", ReturnFirstArgType, ThreeArithmetic);
    _intrinstics["LERP"] = DeclareTemplateFunction(L"lerp", ReturnFirstArgType, ThreeArithmetic);

    std::array<VarConceptDecl*, 1> OneArithmeticVec = { ArthmeticVectorFamily };
    _intrinstics["REDUCE_SUM"] = DeclareTemplateFunction(L"reduce_sum", FloatType, OneArithmeticVec);
    _intrinstics["REDUCE_PRODUCT"] = DeclareTemplateFunction(L"reduce_product", FloatType, OneArithmeticVec);
    _intrinstics["REDUCE_MIN"] = DeclareTemplateFunction(L"reduce_min", FloatType, OneArithmeticVec);
    _intrinstics["REDUCE_MAX"] = DeclareTemplateFunction(L"reduce_max", FloatType, OneArithmeticVec);

    std::array<VarConceptDecl*, 1> OneBoolFamily = { BoolFamily };
    _intrinstics["ALL"] = DeclareTemplateFunction(L"all", BoolType, OneBoolFamily);
    _intrinstics["ANY"] = DeclareTemplateFunction(L"any", BoolType, OneBoolFamily);

    std::array<VarConceptDecl*, 1> OneIntFamily = { IntFamily };
    _intrinstics["CLZ"] = DeclareTemplateFunction(L"clz", ReturnFirstArgType, OneIntFamily);
    _intrinstics["CTZ"] = DeclareTemplateFunction(L"ctz", ReturnFirstArgType, OneIntFamily);
    _intrinstics["POPCOUNT"] = DeclareTemplateFunction(L"popcount", ReturnFirstArgType, OneIntFamily);
    _intrinstics["REVERSE"] = DeclareTemplateFunction(L"reverse", ReturnFirstArgType, OneIntFamily);

    std::array<VarConceptDecl*, 1> OneFloatFamily = { FloatFamily };
    _intrinstics["SIN"] = DeclareTemplateFunction(L"sin", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["SINH"] = DeclareTemplateFunction(L"sinh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["COS"] = DeclareTemplateFunction(L"cos", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["COSH"] = DeclareTemplateFunction(L"cosh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ATAN"] = DeclareTemplateFunction(L"atan", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ATANH"] = DeclareTemplateFunction(L"atanh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["TAN"] = DeclareTemplateFunction(L"tan", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["TANH"] = DeclareTemplateFunction(L"tanh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ACOS"] = DeclareTemplateFunction(L"acos", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ACOSH"] = DeclareTemplateFunction(L"acosh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ASIN"] = DeclareTemplateFunction(L"asin", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ASINH"] = DeclareTemplateFunction(L"asinh", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["EXP"] = DeclareTemplateFunction(L"exp", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["EXP2"] = DeclareTemplateFunction(L"exp2", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["LOG"] = DeclareTemplateFunction(L"log", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["LOG2"] = DeclareTemplateFunction(L"log2", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["LOG10"] = DeclareTemplateFunction(L"log10", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["EXP10"] = DeclareTemplateFunction(L"exp10", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["SQRT"] = DeclareTemplateFunction(L"sqrt", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["RSQRT"] = DeclareTemplateFunction(L"rsqrt", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["CEIL"] = DeclareTemplateFunction(L"ceil", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["FLOOR"] = DeclareTemplateFunction(L"floor", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["FRACT"] = DeclareTemplateFunction(L"fract", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["TRUNC"] = DeclareTemplateFunction(L"trunc", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ROUND"] = DeclareTemplateFunction(L"round", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["LENGTH"] = DeclareTemplateFunction(L"length", FloatType, OneFloatFamily);
    _intrinstics["SATURATE"] = DeclareTemplateFunction(L"saturate", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["DDX"] = DeclareTemplateFunction(L"ddx", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["DDY"] = DeclareTemplateFunction(L"ddy", ReturnFirstArgType, OneFloatFamily);
    _intrinstics["ISINF"] = DeclareTemplateFunction(L"is_inf", ReturnBoolVecWithSameDim, OneFloatFamily);
    _intrinstics["ISNAN"] = DeclareTemplateFunction(L"is_nan", ReturnBoolVecWithSameDim, OneFloatFamily);

    std::array<VarConceptDecl*, 2> TwoFloatFamily = { FloatFamily, FloatFamily };
    _intrinstics["POW"] = DeclareTemplateFunction(L"pow", ReturnFirstArgType, TwoFloatFamily);
    _intrinstics["COPYSIGN"] = DeclareTemplateFunction(L"copysign", ReturnFirstArgType, TwoFloatFamily);
    _intrinstics["ATAN2"] = DeclareTemplateFunction(L"atan2", ReturnFirstArgType, TwoFloatFamily);
    _intrinstics["STEP"] = DeclareTemplateFunction(L"step", ReturnFirstArgType, TwoFloatFamily);

    std::array<VarConceptDecl*, 3> ThreeFloatFamily = { FloatFamily, FloatFamily, FloatFamily };
    _intrinstics["FMA"] = DeclareTemplateFunction(L"fma", ReturnFirstArgType, ThreeFloatFamily);
    _intrinstics["SMOOTHSTEP"] = DeclareTemplateFunction(L"smoothstep", ReturnFirstArgType, ThreeFloatFamily);

    std::array<VarConceptDecl*, 1> OneFloatVector = { FloatVector };
    _intrinstics["NORMALIZE"] = DeclareTemplateFunction(L"normalize", ReturnFirstArgType, OneFloatVector);
    _intrinstics["LENGTH_SQUARED"] = DeclareTemplateFunction(L"length_squared", FloatType, OneFloatVector);

    std::array<VarConceptDecl*, 2> TwoFloatVec = { FloatVector, FloatVector };
    _intrinstics["DOT"] = DeclareTemplateFunction(L"dot", FloatType, TwoFloatVec);
    _intrinstics["CROSS"] = DeclareTemplateFunction(L"cross", ReturnFirstArgType, TwoFloatVec);

    std::array<VarConceptDecl*, 3> ThreeFloat3 = { FloatVector3D, FloatVector3D, FloatVector3D };
    _intrinstics["FACEFORWARD"] = DeclareTemplateFunction(L"faceforward", Float3Type, ThreeFloat3);
    std::array<VarConceptDecl*, 2> TwoFloat3 = { FloatVector3D, FloatVector3D };
    _intrinstics["REFLECT"] = DeclareTemplateFunction(L"reflect", Float3Type, TwoFloat3);

    std::array<VarConceptDecl*, 1> OneMatrix = { MatrixFamily };
    _intrinstics["TRANSPOSE"] = DeclareTemplateFunction(L"transpose", ReturnFirstArgType, OneMatrix);
    _intrinstics["DETERMINANT"] = DeclareTemplateFunction(L"determinant", ReturnFirstArgType, OneMatrix);
    _intrinstics["INVERSE"] = DeclareTemplateFunction(L"inverse", ReturnFirstArgType, OneMatrix);

    std::array<VarConceptDecl*, 3> SelectParams = { ValueFamily, ValueFamily, BoolFamily };
    _intrinstics["SELECT"] = DeclareTemplateFunction(L"select", ReturnFirstArgType, SelectParams);

    std::array<VarConceptDecl*, 2> BufferReadParams = { BufferFamily, IntScalar };
    _intrinstics["BUFFER_READ"] = DeclareTemplateFunction(L"buffer_read", 
        [=](auto pts) { 
            return &dynamic_cast<const StructuredBufferTypeDecl*>(pts[0])->element(); 
        }, BufferReadParams);

    std::array<VarConceptDecl*, 3> BufferWriteParams = { BufferFamily, IntScalar, ValueFamily };
    _intrinstics["BUFFER_WRITE"] = DeclareTemplateFunction(L"buffer_write", VoidType, BufferWriteParams);

    auto AtomicReturnSpec = [=](auto pts) {
        if (auto bufferType = dynamic_cast<const StructuredBufferTypeDecl*>(pts[0])) 
            return &bufferType->element();
        return pts[0];
    };
    std::array<VarConceptDecl*, 3> AtomicParams = { AtomicOperableFamily, IntScalar, IntScalar };
    std::array<VarConceptDecl*, 4> CompareExchangeParams = { AtomicOperableFamily, IntScalar, IntScalar, IntScalar };
    _intrinstics["ATOMIC_EXCHANGE"] = DeclareTemplateFunction(L"atomic_exchange", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_COMPARE_EXCHANGE"] = DeclareTemplateFunction(L"atomic_compare_exchange", AtomicReturnSpec, CompareExchangeParams);
    _intrinstics["ATOMIC_FETCH_ADD"] = DeclareTemplateFunction(L"atomic_fetch_add", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_SUB"] = DeclareTemplateFunction(L"atomic_fetch_sub", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_AND"] = DeclareTemplateFunction(L"atomic_fetch_and", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_OR"] = DeclareTemplateFunction(L"atomic_fetch_or", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_XOR"] = DeclareTemplateFunction(L"atomic_fetch_xor", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_MIN"] = DeclareTemplateFunction(L"atomic_fetch_min", AtomicReturnSpec, AtomicParams);
    _intrinstics["ATOMIC_FETCH_MAX"] = DeclareTemplateFunction(L"atomic_fetch_max", AtomicReturnSpec, AtomicParams);

    std::array<VarConceptDecl*, 2> TextureReadParams = { TextureFamily, IntVector };
    _intrinstics["TEXTURE_READ"] = DeclareTemplateFunction(L"texture_read", 
        [=](auto pts) { 
            return &dynamic_cast<const TextureTypeDecl*>(pts[0])->element(); 
        }, TextureReadParams);
    
    std::array<VarConceptDecl*, 3> TextureWriteParams = { TextureFamily, IntVector, Vector4D };
    _intrinstics["TEXTURE_WRITE"] = DeclareTemplateFunction(L"texture_write", VoidType, TextureWriteParams);
    std::array<VarConceptDecl*, 1> TextureSizeParams = { TextureFamily };
    _intrinstics["TEXTURE_SIZE"] = DeclareTemplateFunction(L"texture_size", UInt3Type, TextureSizeParams);

    std::array<VarConceptDecl*, 4> Texture2DSampleParams = { FloatTexture2DFamily, FloatVector2D/*uv*/, IntScalar/*filter*/, IntScalar/*address*/ };
    _intrinstics["TEXTURE2D_SAMPLE"] = DeclareTemplateFunction(L"texture2d_sample", Float4Type, Texture2DSampleParams);
    std::array<VarConceptDecl*, 5> Texture2DSampleLevelParams = { FloatTexture2DFamily, FloatVector2D, FloatScalar, IntScalar, IntScalar };
    _intrinstics["TEXTURE2D_SAMPLE_LEVEL"] = DeclareTemplateFunction(L"texture2d_sample_level", Float4Type, Texture2DSampleLevelParams);
    std::array<VarConceptDecl*, 6> Texture2DSampleGradParams = { FloatTexture2DFamily, FloatVector2D, FloatVector2D, FloatVector2D, IntScalar, IntScalar };
    _intrinstics["TEXTURE2D_SAMPLE_GRAD"] = DeclareTemplateFunction(L"texture2d_sample_grad", Float4Type, Texture2DSampleGradParams);
    std::array<VarConceptDecl*, 7> Texture2DSampleGradLevelParams = { FloatTexture2DFamily, FloatVector2D, FloatVector2D, FloatVector2D, FloatScalar, IntScalar, IntScalar };
    _intrinstics["TEXTURE2D_SAMPLE_GRAD_LEVEL"] = DeclareTemplateFunction(L"texture2d_sample_grad_level", Float4Type, Texture2DSampleGradLevelParams);

    std::array<VarConceptDecl*, 4> Texture3DSampleParams = { FloatTexture3DFamily, FloatVector3D/*uv*/, IntScalar/*filter*/, IntScalar/*address*/ };
    _intrinstics["TEXTURE3D_SAMPLE"] = DeclareTemplateFunction(L"texture3d_sample", Float4Type, Texture3DSampleParams);
    std::array<VarConceptDecl*, 5> Texture3DSampleLevelParams = { FloatTexture3DFamily, FloatVector3D, FloatScalar, IntScalar, IntScalar };
    _intrinstics["TEXTURE3D_SAMPLE_LEVEL"] = DeclareTemplateFunction(L"texture3d_sample_level", Float4Type, Texture3DSampleLevelParams);
    std::array<VarConceptDecl*, 6> Texture3DSampleGradParams = { FloatTexture3DFamily, FloatVector3D, FloatVector3D, FloatVector3D, IntScalar, IntScalar };
    _intrinstics["TEXTURE3D_SAMPLE_GRAD"] = DeclareTemplateFunction(L"texture3d_sample_grad", Float4Type, Texture3DSampleGradParams);
    std::array<VarConceptDecl*, 7> Texture3DSampleGradLevelParams = { FloatTexture3DFamily, FloatVector3D, FloatVector3D, FloatVector3D, FloatScalar, IntScalar, IntScalar };
    _intrinstics["TEXTURE3D_SAMPLE_GRAD_LEVEL"] = DeclareTemplateFunction(L"texture3d_sample_grad_level", Float4Type, Texture3DSampleGradLevelParams);

    std::array<VarConceptDecl*, 3> Sample2DParams = { SamplerFamily, Texture2DFamily, FloatVector };
    _intrinstics["SAMPLE2D"] = DeclareTemplateFunction(L"sample2d", [this](auto pts) {
        auto pt = &dynamic_cast<const Texture2DTypeDecl*>(pts[1])->element();
        if (pt == FloatType) return Float4Type;
        if (pt == HalfType) return Half4Type;
        if (pt == IntType) return Int4Type;
        if (pt == UIntType) return UInt4Type;
        if (pt == BoolType) return Bool4Type;
        return (const TypeDecl*)nullptr;
    }, Sample2DParams);

    // Declare Ray tracing related types
    auto RayType = DeclareBuiltinType(L"Ray", 32, 16);
    auto CommittedHitType = DeclareBuiltinType(L"CommittedHit", 24, 4);
    auto TriangleHitType = DeclareBuiltinType(L"TriangleHit", 20, 4);
    auto ProceduralHitType = DeclareBuiltinType(L"ProceduralHit", 8, 4);

    // RayQuery family concept
    auto RayQueryFamily = DeclareVarConcept(L"RayQueryFamily", 
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::RayQueryTypeDecl*>(type) != nullptr);
        });

    // TraceRayInline intrinsic
    auto AccelFamily = DeclareVarConcept(L"AccelFamily",
        [this](EVariableQualifier qualifier, const TypeDecl* type) {
            return (dynamic_cast<const skr::CppSL::AccelTypeDecl*>(type) != nullptr);
        });
    std::array<VarConceptDecl*, 9> TraceRayInlineParams = { RayQueryFamily, AccelFamily, IntScalar, IntScalar, 
        FloatVector3D, FloatScalar, FloatVector3D, FloatScalar, IntScalar };
    _intrinstics["TRACE_RAY_INLINE"] = DeclareTemplateFunction(L"trace_ray_inline", VoidType, TraceRayInlineParams);

    // RayQuery methods as intrinsics - all take the RayQuery as first implicit parameter
    _intrinstics["RAY_QUERY_PROCEED"] = DeclareTemplateMethod(nullptr, L"ray_query_proceed", BoolType, {});
    _intrinstics["RAY_QUERY_IS_TRIANGLE_CANDIDATE"] = DeclareTemplateMethod(nullptr, L"ray_query_is_triangle_candidate", BoolType, {});
    _intrinstics["RAY_QUERY_IS_PROCEDURAL_CANDIDATE"] = DeclareTemplateMethod(nullptr, L"ray_query_is_procedural_candidate", BoolType, {});
    _intrinstics["RAY_QUERY_WORLD_SPACE_RAY"] = DeclareTemplateMethod(nullptr, L"ray_query_world_space_ray", RayType, {});
    _intrinstics["RAY_QUERY_PROCEDURAL_CANDIDATE_HIT"] = DeclareTemplateMethod(nullptr, L"ray_query_procedural_candidate_hit", ProceduralHitType, {});
    _intrinstics["RAY_QUERY_TRIANGLE_CANDIDATE_HIT"] = DeclareTemplateMethod(nullptr, L"ray_query_triangle_candidate_hit", TriangleHitType, {});
    _intrinstics["RAY_QUERY_COMMITTED_HIT"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_hit", CommittedHitType, {});
    _intrinstics["RAY_QUERY_COMMIT_TRIANGLE"] = DeclareTemplateMethod(nullptr, L"ray_query_commit_triangle", VoidType, {});
    _intrinstics["RAY_QUERY_TERMINATE"] = DeclareTemplateMethod(nullptr, L"ray_query_terminate", VoidType, {});

    // commit_procedural with float parameter
    std::array<VarConceptDecl*, 1> CommitProceduralParams = { FloatScalar };
    _intrinstics["RAY_QUERY_COMMIT_PROCEDURAL"] = DeclareTemplateMethod(nullptr, L"ray_query_commit_procedural", VoidType, CommitProceduralParams);

    _intrinstics["WAVE_IS_FIRST_ACTIVE_LANE"] = DeclareTemplateFunction(L"wave_is_first_active_lane", BoolType, {});
    _intrinstics["WAVE_ACTIVE_ALL_EQUAL"] = DeclareTemplateFunction(L"wave_active_all_equal", ReturnBoolVecWithSameDim, OneArithmeticVec);
    _intrinstics["WAVE_ACTIVE_BIT_AND"] = DeclareTemplateFunction(L"wave_active_bit_and", ReturnFirstArgType, OneIntFamily);
    _intrinstics["WAVE_ACTIVE_BIT_OR"] = DeclareTemplateFunction(L"wave_active_bit_or", ReturnFirstArgType, OneIntFamily);
    _intrinstics["WAVE_ACTIVE_BIT_XOR"] = DeclareTemplateFunction(L"wave_active_bit_xor", ReturnFirstArgType, OneIntFamily);
    _intrinstics["WAVE_ACTIVE_COUNT_BITS"] = DeclareTemplateFunction(L"wave_active_count_bits", UIntType, OneBoolFamily);
    _intrinstics["WAVE_ACTIVE_MAX"] = DeclareTemplateFunction(L"wave_active_max", ReturnFirstArgType, OneArithmetic);
    _intrinstics["WAVE_ACTIVE_MIN"] = DeclareTemplateFunction(L"wave_active_min", ReturnFirstArgType, OneArithmetic);
    _intrinstics["WAVE_ACTIVE_PRODUCT"] = DeclareTemplateFunction(L"wave_active_product", ReturnFirstArgType, OneArithmetic);
    _intrinstics["WAVE_ACTIVE_SUM"] = DeclareTemplateFunction(L"wave_active_sum", ReturnFirstArgType, OneArithmetic);
    _intrinstics["WAVE_ACTIVE_ALL"] = DeclareTemplateFunction(L"wave_active_all", BoolType, OneBoolFamily);
    _intrinstics["WAVE_ACTIVE_ANY"] = DeclareTemplateFunction(L"wave_active_any", BoolType, OneBoolFamily);
    _intrinstics["WAVE_ACTIVE_BIT_MASK"] = DeclareTemplateFunction(L"wave_active_bit_mask", UInt4Type, OneBoolFamily);
    _intrinstics["WAVE_PREFIX_COUNT_BITS"] = DeclareTemplateFunction(L"wave_prefix_count_bits", UIntType, OneBoolFamily);
    _intrinstics["WAVE_PREFIX_PRODUCT"] = DeclareTemplateFunction(L"wave_prefix_product", ReturnFirstArgType, OneArithmetic);
    _intrinstics["WAVE_PREFIX_SUM"] = DeclareTemplateFunction(L"wave_prefix_sum", ReturnFirstArgType, OneArithmetic);

    // _intrinstics["WAVE_READ_LANE"] = DeclareTemplateFunction(L"wave_read_lane", ReturnFirstArgType, WarpReadParams);
    // _intrinstics["WAVE_READ_FIRST_ACTIVE_LANE"] = DeclareTemplateFunction(L"wave_read_first_active_lane", ReturnFirstArgType, OnePrimitiveFamily);

    _intrinstics["SYNCHRONIZE_BLOCK"] = DeclareTemplateFunction(L"sync_block", VoidType, {});
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