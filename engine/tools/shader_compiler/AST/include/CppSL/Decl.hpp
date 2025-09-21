#pragma once
#include <functional>
#include "Stmt.hpp"

namespace skr::CppSL {

struct AST;
struct Attr;
struct Expr;
struct TypeDecl;
struct FunctionDecl;
struct FieldDecl;
struct MethodDecl;
struct ConstructorDecl;
struct ConstantExpr;
struct GlobalVarDecl;

struct Decl
{
public:
    virtual String dump() const;
    virtual ~Decl() = default;
    virtual const Stmt* body() const = 0;
    virtual DeclRefExpr* ref() const;

    const AST& ast() const { return *_ast; }
    std::span<Attr* const> attrs() const { return _attrs; }
    void add_attr(Attr* attr);
    void add_attrs(std::span<Attr* const> attrs) { for (auto attr : attrs) add_attr(attr); }

protected:
    friend struct AST;
    Decl(AST& ast);
    const AST* _ast = nullptr;
    std::vector<Attr*> _attrs;
};

struct NamedDecl : public Decl
{
public:
    const Name& name() const { return _name; }

protected:
    NamedDecl(AST& ast, const Name& name);
    Name _name = L"__INVALID_DECL__";
};

struct VarDecl : public NamedDecl
{
public:
    const TypeDecl& type() const { return *_type; }
    Expr* initializer() const { return _initializer; }
    EVariableQualifier qualifier() const { return _qualifier; }
    const Stmt* body() const override;
    virtual bool is_global() const;

protected:
    friend struct AST;    
    VarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer = nullptr);
    EVariableQualifier _qualifier = EVariableQualifier::None; // the qualifier of the variable (e.g., const, in, out, etc.)
    const TypeDecl* _type = nullptr;
    Expr* _initializer = nullptr;
};

struct FieldDecl : public NamedDecl
{
public:
    const TypeDecl& type() const { return *_type; }
    const Size size() const;
    const Size alignment() const;
    const Stmt* body() const override;

protected:
    friend struct AST;    
    FieldDecl(AST& ast, const Name& _name, const TypeDecl* type);
    const TypeDecl* _type = nullptr;
};

struct TypeDecl : public NamedDecl
{
public:
    bool is_builtin() const { return _is_builtin; }
    bool is_array() const;
    bool is_vector() const;
    bool is_matrix() const;
    bool is_resource() const;
    bool is_empty() const { return _fields.size() == 0 && _methods.size() == 0 && _ctors.size() == 0; }

    const Size size() const  { return _size; }
    const Size alignment() const { return _alignment; }
    const auto& fields() const { return _fields; }
    const auto& methods() const { return _methods; }
    const auto& ctors() const { return _ctors; }
    const Stmt* body() const override;

    void add_field(FieldDecl* field);
    void add_method(MethodDecl* method);
    void add_ctor(ConstructorDecl* ctor);

    FieldDecl* get_field(const Name& name) const;
    MethodDecl* get_method(const Name& name) const;

protected:
    friend struct AST;
    TypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment = 4, std::span<FieldDecl*> fields = {}, bool is_builtin = true);
    TypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin = false);

    const bool _is_builtin = true;
    Size _size = 0;
    Size _alignment = 0;
    std::vector<FieldDecl*> _fields;
    std::vector<MethodDecl*> _methods;
    std::vector<ConstructorDecl*> _ctors;
};

struct NamespaceDecl : public NamedDecl
{
public:
    const auto& nested() const { return _nested; }
    const auto& types() const { return _types; }
    const auto& functions() const { return _functions; }
    const auto& global_vars() const { return _global_vars; }
    
    void add_nested(NamespaceDecl* nested);
    NamespaceDecl* parent() const { return _parent; }

    void add_type(TypeDecl* type);
    void add_function(FunctionDecl* function);
    void add_global_var(GlobalVarDecl* var);
    
    bool has_content() const; // Check if namespace has any meaningful content
    String dump() const override;
    const Stmt* body() const override;

protected:
    friend struct AST;
    NamespaceDecl(AST& ast, const Name& name, NamespaceDecl* parent = nullptr);
    std::vector<NamespaceDecl*> _nested; // nested namespaces
    std::vector<TypeDecl*> _types; // types declared in this namespace
    std::vector<FunctionDecl*> _functions; // functions declared in this namespace
    std::vector<GlobalVarDecl*> _global_vars; // global variables declared in this namespace
    NamespaceDecl* _parent = nullptr; // parent namespace
};

struct ValueTypeDecl : public TypeDecl
{
protected:
    ValueTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment = 4, std::span<FieldDecl*> fields = {}, bool is_builtin = true);
    ValueTypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin = false);
};

struct ScalarTypeDecl : public ValueTypeDecl
{
protected:
    friend struct AST;
    ScalarTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment);
};

struct VectorTypeDecl : public ValueTypeDecl
{
public:
    const auto& element_type() const { return _element; }
    uint32_t count() const { return _count; }

protected:
    friend struct AST;
    VectorTypeDecl(AST& ast, const TypeDecl* element, uint32_t count, uint32_t alignment);
    const TypeDecl* _element = nullptr; 
    uint32_t _count = 0; 
};

struct MatrixTypeDecl : public ValueTypeDecl
{
public:
    const auto& columns() const { return _n; }
    const auto& rows() const { return _n; }
    const auto& element_type() const { return _element; }

protected:
    friend struct AST;
    MatrixTypeDecl(AST& ast, const TypeDecl* element, uint32_t n, uint32_t alignment);
    const TypeDecl* _element = nullptr; 
    uint32_t _n = 0;
};

struct StructureTypeDecl : public ValueTypeDecl
{
protected:
    friend struct AST;
    StructureTypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin = false);
    StructureTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment = 4, std::span<FieldDecl*> fields = {}, bool is_builtin = true);
};

struct ArrayTypeDecl : public ValueTypeDecl
{
public:
    ArrayFlags flags() const { return _flags; }
    const TypeDecl* element_type() const { return _element; }
    const auto count() const { return _count; }

protected:
    friend struct AST;
    ArrayTypeDecl(AST& ast, const TypeDecl* element, uint32_t count, ArrayFlags flags);
    const TypeDecl* _element = nullptr;
    ArrayFlags _flags = ArrayFlags::None;
    uint32_t _count = 0;
};

struct RayQueryTypeDecl : public TypeDecl
{
public:
    RayQueryFlags flags() const { return _flags; }

protected:
    friend struct AST;
    RayQueryTypeDecl(AST& ast, RayQueryFlags flags);
    RayQueryFlags _flags = RayQueryFlags::None; // flags for the ray query type
};

struct ResourceTypeDecl : public TypeDecl
{
protected:
    ResourceTypeDecl(AST& ast, const String& name);
};

struct SamplerDecl : public ResourceTypeDecl
{
protected:
    friend struct AST;
    SamplerDecl(AST& ast);
};

struct AccelTypeDecl : public ResourceTypeDecl
{
protected:
    friend struct AST;
    AccelTypeDecl(AST& ast);
};

struct BufferTypeDecl : public ResourceTypeDecl
{
public:
    const auto flags() const { return _flags; }

protected:
    BufferTypeDecl(AST& ast, const String& name, BufferFlags flags);
    BufferFlags _flags;
};

struct ConstantBufferTypeDecl : public BufferTypeDecl
{
public:
    const auto element_type() const { return _element; }
    
protected:
    friend struct AST;
    ConstantBufferTypeDecl(AST& ast, const TypeDecl* element);
    const TypeDecl* _element = nullptr;
};

struct ByteBufferTypeDecl : public BufferTypeDecl
{
protected:
    friend struct AST;
    ByteBufferTypeDecl(AST& ast, BufferFlags flags);
};

struct TexelBufferTypeDecl : public BufferTypeDecl
{
public:
    const TypeDecl& element_type() const { return *_element; }
    const Size element_size() const { return _element->size(); }
    const Size element_alignment() const { return _element->alignment(); }

protected:
    friend struct AST;
    TexelBufferTypeDecl(AST& ast, const TypeDecl* element, BufferFlags flags);
    const TypeDecl* _element = nullptr; // the type of elements in the buffer
};

struct StructuredBufferTypeDecl : public BufferTypeDecl
{
public:
    const TypeDecl& element_type() const { return *_element; }
    const Size element_size() const { return _element->size(); }
    const Size element_alignment() const { return _element->alignment(); }

protected:
    friend struct AST;
    StructuredBufferTypeDecl(AST& ast, const TypeDecl* element, BufferFlags flags);
    const TypeDecl* _element = nullptr; // the type of elements in the buffer
};

struct TextureTypeDecl : public ResourceTypeDecl
{
public:
    const TypeDecl& element_type() const { return *_element; }
    const auto flags() const { return _flags; }
    bool is_array() const { return _is_array; }

protected:
    TextureTypeDecl(AST& ast, const String& name, const TypeDecl* element, TextureFlags flags);
    const TypeDecl* _element = nullptr; // the type of elements in the buffer
    TextureFlags _flags;
    bool _is_array = false;
};

struct Texture1DTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture1DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct Texture2DTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture2DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct Texture1DArrayTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture1DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct Texture2DArrayTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture2DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct Texture3DTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture3DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct Texture3DArrayTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    Texture3DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct TextureCubeTypeDecl : public TextureTypeDecl
{
protected:
    friend struct AST;
    TextureCubeTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags);
};

struct GlobalVarDecl : public VarDecl
{
public:
    const TypeDecl& type() const { return *_type; }
    virtual bool is_global() const override;
    
protected:
    friend struct AST;
    GlobalVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name, Expr* initializer);
};

struct ParamVarDecl : public VarDecl
{
public:
    const TypeDecl& type() const { return *_type; }

protected:
    friend struct AST;    
    ParamVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name);
};

struct VarConceptDecl : public NamedDecl
{
public:
    const Stmt* body() const override;
    virtual bool validate(EVariableQualifier qualifier, const TypeDecl* type) const = 0;

protected:
    friend struct AST;
    VarConceptDecl(AST& ast, const Name& name);
};

struct FunctionDecl : public NamedDecl
{
public:
    const TypeDecl* return_type() const;
    const std::span<const ParamVarDecl* const> parameters() const;
    const Stmt* body() const override { return _body; }
    CppSL::ShaderStage stage() const;
    bool is_static() const { return _is_static; }
    void set_static(bool is_static_) { _is_static = is_static_; }

protected:
    friend struct AST;
    FunctionDecl(AST& ast, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body);
    bool _is_static = false;
    const CompoundStmt* _body = nullptr;
    const TypeDecl* _return_type = nullptr;
    std::vector<const ParamVarDecl*> _parameters;
};

struct MethodDecl : public FunctionDecl
{
public:
    const TypeDecl* owner_type() const { return _owner; }

    void set_const(bool is_const) { _const = is_const; }
    bool is_const() const { return _const; }

protected:
    friend struct AST;
    MethodDecl(AST& ast, TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body);
    const TypeDecl* _owner = nullptr; // the type that owns this method
    bool _const = false;
};

struct ConstructorDecl : public MethodDecl
{
public:
    inline static const Name kSymbolName = L"__SSL_CTOR__";
    
    // Member initializer for constructor initialization list
    struct MemberInit {
        const FieldDecl* field;
        const Expr* init_expr;
    };
    
    std::span<const MemberInit> member_inits() const { return _member_inits; }
    void add_member_init(const FieldDecl* field, const Expr* init_expr);

protected:
    friend struct AST;
    ConstructorDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, const CompoundStmt* body);
    
    std::vector<MemberInit> _member_inits;
};

// Template callable that can represent both function and method templates
struct TemplateCallableDecl : public NamedDecl
{
public:
    using ReturnTypeSpecializer = std::function<const TypeDecl*(std::span<const TypeDecl* const>)>;
    
    const Stmt* body() const override { return nullptr; } // Templates have no body
    
    // Get the return type for specific argument types (may depend on arguments)
    virtual const TypeDecl* get_return_type_for(std::span<const TypeDecl* const> arg_types) const = 0;
    
    // Get parameter concepts for validation
    std::span<const VarConceptDecl* const> parameter_concepts() const { return _parameter_concepts; }
    
    // Validate if a call with given argument types and qualifiers matches this template
    bool can_call_with(std::span<const TypeDecl* const> arg_types, 
                      std::span<const EVariableQualifier> arg_qualifiers) const;
    
    // For methods, returns the owner type; for functions, returns nullptr
    const TypeDecl* owner_type() const { return _owner; }
    bool is_method() const { return _owner != nullptr; }
    
    // Generate a specialized function/method for given argument types
    FunctionDecl* specialize_for(std::span<const TypeDecl* const> arg_types, 
                                std::span<const EVariableQualifier> arg_qualifiers,
                                const TypeDecl* ret_spec = nullptr) const;

protected:
    friend struct AST;
    // Constructor for template function
    TemplateCallableDecl(AST& ast, const Name& name, std::span<const VarConceptDecl* const> param_concepts);
    // Constructor for template method
    TemplateCallableDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const VarConceptDecl* const> param_concepts);
    
    const TypeDecl* _owner = nullptr;
    std::vector<const VarConceptDecl*> _parameter_concepts;
};

// Specialized function generated from template (inherits from FunctionDecl)
struct SpecializedFunctionDecl : public FunctionDecl
{
public:
    const TemplateCallableDecl* template_decl() const { return _template; }
    
    // Override to indicate this is a specialization
    bool is_template_specialization() const { return true; }

protected:
    friend struct AST;
    friend struct TemplateCallableDecl;
    SpecializedFunctionDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                           const TypeDecl* override_return_type,
                           std::span<const TypeDecl* const> arg_types,
                           std::span<const EVariableQualifier> arg_qualifiers);
    
    const TemplateCallableDecl* _template = nullptr;
};

// Specialized method generated from template (inherits from MethodDecl)
struct SpecializedMethodDecl : public MethodDecl
{
public:
    const TemplateCallableDecl* template_decl() const { return _template; }
    
    // Override to indicate this is a specialization
    bool is_template_specialization() const { return true; }

protected:
    friend struct AST;
    friend struct TemplateCallableDecl;
    SpecializedMethodDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                         const TypeDecl* override_return_type,
                         std::span<const TypeDecl* const> arg_types,
                         std::span<const EVariableQualifier> arg_qualifiers);
    
    const TemplateCallableDecl* _template = nullptr;
};

} // namespace skr::CppSL