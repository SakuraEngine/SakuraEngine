#include "CppSL/CppSLAST.hpp"
#include "CppSL/Decl.hpp"
#include <format>

namespace skr::CppSL {

Decl::Decl(AST& ast) 
    : _ast(&ast) 
{

}

void Decl::add_attr(Attr* attr)
{
    if (attr)
    {
        _attrs.emplace_back(attr);
    }
}

DeclRefExpr* Decl::ref() const
{
    return const_cast<AST*>(_ast)->Ref(this);
}

NamedDecl::NamedDecl(AST& ast, const Name& name)
    : Decl(ast), _name(name)
{

}

const Stmt* VarDecl::body() const
{
    return nullptr;
}

VarDecl::VarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name, Expr* initializer)
    : NamedDecl(ast, name), _qualifier(qualifier), _type(type), _initializer(initializer)
{

}

bool VarDecl::is_global() const
{
    return false; // Default implementation, can be overridden
}

FieldDecl::FieldDecl(AST& ast, const Name& name, const TypeDecl* type)
    : NamedDecl(ast, name), _type(type)
{

}

const Size FieldDecl::size() const
{
    return _type->size();
}

const Size FieldDecl::alignment() const
{
    return _type->alignment();
}

const Stmt* FieldDecl::body() const
{
    return nullptr;
}

TypeDecl::TypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment, std::span<FieldDecl*> fields, bool is_builtin)
    : NamedDecl(ast, name), _is_builtin(is_builtin), _size(size), _alignment(alignment), _fields(fields.begin(), fields.end())
{

}

TypeDecl::TypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
    : NamedDecl(ast, name), _is_builtin(is_builtin)
{
    _fields.reserve(_fields.size());
    for (const auto& field : fields)
    {
        _fields.emplace_back(field);
        _size += field->size();
        _alignment = std::max(_alignment, field->alignment());
    }
}

const Stmt* TypeDecl::body() const
{
    return nullptr;
}

bool TypeDecl::is_array() const
{
    return dynamic_cast<const ArrayTypeDecl*>(this) != nullptr;
}

bool TypeDecl::is_vector() const
{
    return dynamic_cast<const VectorTypeDecl*>(this) != nullptr;
}

bool TypeDecl::is_matrix() const
{
    return dynamic_cast<const MatrixTypeDecl*>(this) != nullptr;
}

bool TypeDecl::is_resource() const
{
    if (auto as_array = dynamic_cast<const ArrayTypeDecl*>(this))
        return as_array->element_type()->is_resource();
    else
        return dynamic_cast<const ResourceTypeDecl*>(this) != nullptr;
}

void TypeDecl::add_field(FieldDecl* field)
{
    _fields.emplace_back(field);
    _size += field->size();
    _alignment = std::max(_alignment, field->alignment());
}

void TypeDecl::add_method(MethodDecl* method)
{
    assert(method != nullptr && "Method cannot be null");
    assert(dynamic_cast<ConstructorDecl*>(method) == nullptr && "Cannot add constructor as method");
    
    _methods.emplace_back(method);
}

void TypeDecl::add_ctor(ConstructorDecl* ctor)
{
    assert(ctor != nullptr && "Constructor cannot be null");
    _ctors.emplace_back(ctor);
}

FieldDecl* TypeDecl::get_field(const Name& name) const
{
    auto found = std::find_if(_fields.begin(), _fields.end(), [&](const FieldDecl* field) { return field->name() == name; });
    if (found != _fields.end())
        return *found;
    return nullptr;
}

MethodDecl* TypeDecl::get_method(const Name& name) const
{
    auto found = std::find_if(_methods.begin(), _methods.end(), [&](const MethodDecl* method) { return method->name() == name; });
    if (found != _methods.end())
        return *found;
    return nullptr;
}

ValueTypeDecl::ValueTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment, std::span<FieldDecl*> fields, bool is_builtin)
    : TypeDecl(ast, name, size, alignment, fields, is_builtin)
{

}

ValueTypeDecl::ValueTypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
    : TypeDecl(ast, name, fields, is_builtin)
{

}

StructureTypeDecl::StructureTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment, std::span<FieldDecl*> fields, bool is_builtin)
    : ValueTypeDecl(ast, name, size, alignment, fields, is_builtin)
{

}

StructureTypeDecl::StructureTypeDecl(AST& ast, const Name& name, std::span<FieldDecl*> fields, bool is_builtin)
    : ValueTypeDecl(ast, name, fields, is_builtin)
{

}

ScalarTypeDecl::ScalarTypeDecl(AST& ast, const Name& name, uint32_t size, uint32_t alignment)
    : ValueTypeDecl(ast, name, size, alignment, {}, true)
{

}

VectorTypeDecl::VectorTypeDecl(AST& ast, const TypeDecl* element, uint32_t count, uint32_t alignment)
    : ValueTypeDecl(ast, std::format(L"{}{}", element->name(), count), element->size() * count, alignment, {}, true),
    _element(element), _count(count)
{

}

MatrixTypeDecl::MatrixTypeDecl(AST& ast, const TypeDecl* element, uint32_t x, uint32_t y, uint32_t alignment)
    : ValueTypeDecl(ast, std::format(L"{}{}x{}", element->name(), x, y), element->size() * x * y, alignment, {}, true),
    _element(element), _x(x), _y(y)
{

}

ArrayTypeDecl::ArrayTypeDecl(AST& ast, const TypeDecl* element, uint32_t count, ArrayFlags flags)
    : ValueTypeDecl(ast, std::format(L"array<{}, {}>", element->name(), count), element->size() * count, element->alignment(), {}, true),
    _element(element), _flags(flags), _count(count)
{

}

RayQueryTypeDecl::RayQueryTypeDecl(AST& ast, RayQueryFlags flags)
    : TypeDecl(ast, std::format(L"RayQuery<{}>", (uint32_t)flags), 0, 0, {}, true), _flags(flags)
{

}

ResourceTypeDecl::ResourceTypeDecl(AST& ast, const String& name)
    : TypeDecl(ast, name, 0, 0, {}, true)
{
    _size = 8;
}

SamplerDecl::SamplerDecl(AST& ast)
    : ResourceTypeDecl(ast, L"SamplerState")
{

}

AccelTypeDecl::AccelTypeDecl(AST& ast)
    : ResourceTypeDecl(ast, L"AccelerationStructure")
{

}

BufferTypeDecl::BufferTypeDecl(AST& ast, const String& name, BufferFlags flags)
    : ResourceTypeDecl(ast, name), _flags(flags)
{

}

ConstantBufferTypeDecl::ConstantBufferTypeDecl(AST& ast, const TypeDecl* element)
    : BufferTypeDecl(ast, std::format(L"ConstantBuffer<{}>", element->name()), BufferFlags::None), _element(element)
{

}

ByteBufferTypeDecl::ByteBufferTypeDecl(AST& ast, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}ByteAddressBuffer", has_flag(flags, BufferFlags::ReadWrite) ? L"RW" : L""), flags)
{

}

TexelBufferTypeDecl::TexelBufferTypeDecl(AST& ast, const TypeDecl* element, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}TexelBuffer<{}>", has_flag(flags, BufferFlags::ReadWrite) ? L"RW" : L"", element->name()), flags), _element(element)
{

}

StructuredBufferTypeDecl::StructuredBufferTypeDecl(AST& ast, const TypeDecl* element, BufferFlags flags)
    : BufferTypeDecl(ast, std::format(L"{}StructuredBuffer<{}>", has_flag(flags, BufferFlags::ReadWrite) ? L"RW" : L"", element->name()), flags), _element(element)
{

}

TextureTypeDecl::TextureTypeDecl(AST& ast, const String& name, const TypeDecl* element, TextureFlags flags)
    : ResourceTypeDecl(ast, name), _element(element), _flags(flags)
{

}

Texture1DTypeDecl::Texture1DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture1D<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

Texture2DTypeDecl::Texture2DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture2D<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

Texture1DArrayTypeDecl::Texture1DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture1DArray<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{   
    _is_array = true;
}

Texture2DArrayTypeDecl::Texture2DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture2DArray<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{
    _is_array = true;
}

Texture3DTypeDecl::Texture3DTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture3D<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

Texture3DArrayTypeDecl::Texture3DArrayTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}Texture3DArray<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{
    _is_array = true;
}

TextureCubeTypeDecl::TextureCubeTypeDecl(AST& ast, const TypeDecl* element, TextureFlags flags)
    : TextureTypeDecl(ast, std::format(L"{}TextureCube<{}>", has_flag(flags, TextureFlags::ReadWrite) ? L"RW" : L"", element->name()), element, flags)
{

}

GlobalVarDecl::GlobalVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& _name, Expr* initializer)
    : VarDecl(ast, qualifier, type, _name, initializer)
{

}

bool GlobalVarDecl::is_global() const
{
    return true; // Global variables are always global
}

ParamVarDecl::ParamVarDecl(AST& ast, EVariableQualifier qualifier, const TypeDecl* type, const Name& name)
    : VarDecl(ast, qualifier, type, name)
{

}

FunctionDecl::FunctionDecl(AST& ast, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : NamedDecl(ast, name), _return_type(return_type), _body(body)
{
    _parameters.reserve(_parameters.size());
    for (const auto& param : params)
    {
        _parameters.emplace_back(param);
    }
}

const TypeDecl* FunctionDecl::return_type() const 
{ 
    return _return_type; 
}

const std::span<const ParamVarDecl* const> FunctionDecl::parameters() const
{
    return std::span<const ParamVarDecl* const>(_parameters);
}

CppSL::ShaderStage FunctionDecl::stage() const
{
    for (auto attr : attrs())
    {
        if (auto asStage = dynamic_cast<CppSL::StageAttr*>(attr))
        {
            return asStage->stage();
        }
    }
    return CppSL::ShaderStage::None; 
}

MethodDecl::MethodDecl(AST& ast, TypeDecl* owner, const Name& name, const TypeDecl* return_type, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : FunctionDecl(ast, name, return_type, params, body), _owner(owner)
{

}

ConstructorDecl::ConstructorDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const ParamVarDecl* const> params, const CompoundStmt* body)
    : MethodDecl(ast, owner, name, ast.VoidType, params, body)
{

}

void ConstructorDecl::add_member_init(const FieldDecl* field, const Expr* init_expr)
{
    _member_inits.push_back({field, init_expr});
}

VarConceptDecl::VarConceptDecl(AST& ast, const Name& name)
    : NamedDecl(ast, name)
{

}

const Stmt* VarConceptDecl::body() const
{
    return nullptr; // Concepts have no body
}

TemplateCallableDecl::TemplateCallableDecl(AST& ast, const Name& name, std::span<const VarConceptDecl* const> param_concepts)
    : NamedDecl(ast, name), _owner(nullptr)
{
    _parameter_concepts.reserve(param_concepts.size());
    for (auto param_concept : param_concepts) {
        _parameter_concepts.push_back(param_concept);
    }
}

TemplateCallableDecl::TemplateCallableDecl(AST& ast, TypeDecl* owner, const Name& name, std::span<const VarConceptDecl* const> param_concepts)
    : NamedDecl(ast, name), _owner(owner)
{
    _parameter_concepts.reserve(param_concepts.size());
    for (auto param_concept : param_concepts) {
        _parameter_concepts.push_back(param_concept);
    }
}

bool TemplateCallableDecl::can_call_with(std::span<const TypeDecl* const> arg_types, 
                                        std::span<const EVariableQualifier> arg_qualifiers) const
{
    if (arg_types.size() != _parameter_concepts.size() || 
        arg_qualifiers.size() != _parameter_concepts.size()) {
        return false;
    }
      for (size_t i = 0; i < arg_types.size(); ++i) {
        const auto& param_concept = _parameter_concepts[i];
        // Check both type and qualifier constraints in validate method
        if (!param_concept->validate(arg_qualifiers[i], arg_types[i])) {
            return false;
        }
    }
    
    return true;
}

FunctionDecl* TemplateCallableDecl::specialize_for(std::span<const TypeDecl* const> arg_types, std::span<const EVariableQualifier> arg_qualifiers, const TypeDecl* ret_spec) const
{
    if (!can_call_with(arg_types, arg_qualifiers)) {
        return nullptr;
    }
    
    // Create specialized function or method based on whether this template has an owner
    if (is_method()) {
        return new SpecializedMethodDecl(const_cast<AST&>(ast()), this, ret_spec, arg_types, arg_qualifiers);
    } else {
        return new SpecializedFunctionDecl(const_cast<AST&>(ast()), this, ret_spec, arg_types, arg_qualifiers);
    }
}

// SpecializedFunctionDecl implementation
SpecializedFunctionDecl::SpecializedFunctionDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                                                 const TypeDecl* override_return_type, 
                                                 std::span<const TypeDecl* const> arg_types,
                                                 std::span<const EVariableQualifier> arg_qualifiers)
    : FunctionDecl(ast, template_decl->name(), 
    override_return_type ? override_return_type : template_decl->get_return_type_for(arg_types), {}, nullptr), 
    _template(template_decl)
{
    // Create concrete parameters from template concepts and argument types
    _parameters.reserve(arg_types.size());
    auto concepts = template_decl->parameter_concepts();
    
    for (size_t i = 0; i < arg_types.size(); ++i) {
        auto param_name = concepts[i]->name(); // Use concept name as parameter name
        auto param = ast.DeclareParam(arg_qualifiers[i], arg_types[i], param_name);
        _parameters.push_back(param);
    }
}

// SpecializedMethodDecl implementation
SpecializedMethodDecl::SpecializedMethodDecl(AST& ast, const TemplateCallableDecl* template_decl, 
                                             const TypeDecl* override_return_type,
                                             std::span<const TypeDecl* const> arg_types,
                                             std::span<const EVariableQualifier> arg_qualifiers)
    : MethodDecl(ast, const_cast<TypeDecl*>(template_decl->owner_type()), template_decl->name(), 
                 override_return_type ? override_return_type : template_decl->get_return_type_for(arg_types), {}, nullptr), 
      _template(template_decl)
{
    // Create concrete parameters from template concepts and argument types
    _parameters.reserve(arg_types.size());
    auto concepts = template_decl->parameter_concepts();
    
    for (size_t i = 0; i < arg_types.size(); ++i) {
        auto param_name = concepts[i]->name(); // Use concept name as parameter name
        auto param = ast.DeclareParam(arg_qualifiers[i], arg_types[i], param_name);
        _parameters.push_back(param);
    }
}

NamespaceDecl::NamespaceDecl(AST& ast, const Name& name, NamespaceDecl* parent)
    : NamedDecl(ast, name), _parent(parent)
{
}

const Stmt* NamespaceDecl::body() const
{
    return nullptr; // Namespaces have no body in shader context
}

void NamespaceDecl::add_nested(NamespaceDecl* nested)
{
    if (nested)
    {
        // Check if already added to avoid duplicates
        auto it = std::find(_nested.begin(), _nested.end(), nested);
        if (it == _nested.end())
        {
            _nested.emplace_back(nested);
            nested->_parent = this;
        }
    }
}

void NamespaceDecl::add_type(TypeDecl* type)
{
    if (type)
    {
        // Check if already added to avoid duplicates
        auto it = std::find(_types.begin(), _types.end(), type);
        if (it == _types.end())
        {
            _types.emplace_back(type);
        }
    }
}

void NamespaceDecl::add_function(FunctionDecl* function)
{
    if (function)
    {
        // Check if already added to avoid duplicates
        auto it = std::find(_functions.begin(), _functions.end(), function);
        if (it == _functions.end())
        {
            _functions.emplace_back(function);
        }
    }
}

void NamespaceDecl::add_global_var(GlobalVarDecl* var)
{
    if (var)
    {
        // Check if already added to avoid duplicates
        auto it = std::find(_global_vars.begin(), _global_vars.end(), var);
        if (it == _global_vars.end())
        {
            _global_vars.emplace_back(var);
        }
    }
}

bool NamespaceDecl::has_content() const
{
    // Check if has any non-builtin types
    for (const auto& type : _types)
    {
        if (!type->is_builtin())
            return true;
    }
    
    // Check if has any functions
    if (!_functions.empty())
        return true;
    
    // Check if has any global variables
    if (!_global_vars.empty())
        return true;
    
    // Check if any nested namespace has content
    for (const auto& nested : _nested)
    {
        if (nested->has_content())
            return true;
    }
    
    return false;
}

String NamespaceDecl::dump() const
{
    return std::format(L"namespace {} {{ {} nested, {} types, {} functions, {} globals }}",
                       name(), _nested.size(), _types.size(), _functions.size(), _global_vars.size());
}

}