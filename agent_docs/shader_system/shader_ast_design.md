# SakuraEngine ShaderAST 设计与实现文档

## 概述

ShaderAST 是 SakuraEngine 自研的着色器中间表示（IR）系统，作为从 C++ AST 到各种着色器语言（HLSL、MSL、GLSL）的桥梁。系统通过解析带有 `[[callop]]` 等属性标记的 C++ 代码，转换为简化的 Shader AST，再通过专门的代码生成器输出目标着色器代码。

## 设计理念

### 核心目标

1. **简化性**：剔除 C++ 的复杂特性（模板、继承、虚函数等）
2. **GPU 友好**：内置 GPU 内存布局规则和资源模型
3. **跨平台**：统一的中间表示支持多种目标语言
4. **类型安全**：保持强类型系统，便于验证和优化
5. **可扩展**：易于添加新的语言特性和目标平台

### 架构位置

```
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│  C++ 源码   │ ──> │  Clang AST   │ ──> │  ShaderAST   │
└─────────────┘     └──────────────┘     └──────────────┘
                                                  │
                    ┌──────────────────────────────┼──────────────────────────────┐
                    ▼                              ▼                              ▼
            ┌──────────────┐            ┌──────────────┐              ┌──────────────┐
            │     HLSL     │            │     MSL      │              │     GLSL     │
            └──────────────┘            └──────────────┘              └──────────────┘
```

## 核心数据结构

### AST 主类

AST 类是 Shader AST 的核心管理器，负责内置函数注册和代码生成：

```cpp
// tools/LLVMTools/shader_compiler/AST/include/CppSL/CppSLAST.hpp
struct AST {
    // 内置函数查找和特化
    const TemplateCallableDecl* FindIntrinsic(const char* name) const;
    SpecializedFunctionDecl* SpecializeTemplateFunction(const TemplateCallableDecl* template_decl, 
                                                        std::span<const TypeDecl* const> arg_types, 
                                                        std::span<const EVariableQualifier> arg_qualifiers);
    
    // 内置类型实例
    const TypeDecl* VoidType;
    const TypeDecl* BoolType;
    const TypeDecl* FloatType;
    const TypeDecl* IntType;
    const TypeDecl* UIntType;
    
    // 向量和矩阵类型
    const TypeDecl* Float2Type;
    const TypeDecl* Float3Type; 
    const TypeDecl* Float4Type;
    const TypeDecl* Float4x4Type;
    // ... 其他类型
    
private:
    // 内置函数注册表 - 在构造函数中填充
    std::unordered_map<std::string, TemplateCallableDecl*> _intrinsics;
};
```

## callop 属性系统

ShaderAST 使用 `[[callop("NAME")]]` 属性将 C++ 方法映射到着色器内置函数。这是系统的核心机制之一：

```cpp
// engine/LLVMTools/tools/shader_compiler/ShaderSTL/std/raytracing/ray_query.hpp
template <uint32 flags>
struct ray_query {
    // 控制方法
    [[callop("RAY_QUERY_TRACE_RAY_INLINE")]] void trace_ray_inline(acceleration_structure as, uint32 ray_flags, uint32 cull_mask, ray_desc ray);
    [[callop("RAY_QUERY_PROCEED")]] bool proceed();
    [[callop("RAY_QUERY_TERMINATE")]] void terminate();
    
    // 查询方法
    [[callop("RAY_QUERY_COMMITTED_STATUS")]] uint32 committed_status();
    [[callop("RAY_QUERY_COMMITTED_RAY_T")]] float committed_ray_t();
    [[callop("RAY_QUERY_WORLD_RAY_ORIGIN")]] float3 world_ray_origin();
    [[callop("RAY_QUERY_WORLD_RAY_DIRECTION")]] float3 world_ray_direction();
    
    // 几何信息
    [[callop("RAY_QUERY_COMMITTED_TRIANGLE_BARYCENTRICS")]] float2 committed_triangle_barycentrics();
    [[callop("RAY_QUERY_COMMITTED_INSTANCE_INDEX")]] uint32 committed_instance_index();
    [[callop("RAY_QUERY_COMMITTED_GEOMETRY_INDEX")]] uint32 committed_geometry_index();
    [[callop("RAY_QUERY_COMMITTED_PRIMITIVE_INDEX")]] uint32 committed_primitive_index();
    
    // 变换矩阵
    [[callop("RAY_QUERY_COMMITTED_OBJECT_TO_WORLD_3X4")]] float3x4 committed_object_to_world_3x4();
    [[callop("RAY_QUERY_COMMITTED_WORLD_TO_OBJECT_3X4")]] float3x4 committed_world_to_object_3x4();
    
    // 特殊查询（生成比较表达式）
    [[callop("RAY_QUERY_IS_TRIANGLE_CANDIDATE")]] bool is_triangle_candidate();
    [[callop("RAY_QUERY_IS_PROCEDURAL_CANDIDATE")]] bool is_procedural_candidate();
};
```

### 属性映射原理

1. **Clang 解析**：`[[callop("NAME")]]` 被 Clang AST 解析为注解属性
2. **内置函数注册**：在 AST 构造时注册对应的内置函数
3. **代码生成**：HLSLGenerator 根据方法名映射到对应的 HLSL 内置函数

### 内置函数注册机制

系统在 AST 构造时注册所有内置函数：

```cpp
// tools/LLVMTools/shader_compiler/AST/src/AST.cpp (实际代码片段)
AST::AST() {
    // 数学函数
    std::array<VarConceptDecl*, 1> OneFloatFamily = { FloatFamily };
    _intrinsics["SQRT"] = DeclareTemplateFunction(L"sqrt", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["SIN"] = DeclareTemplateFunction(L"sin", ReturnFirstArgType, OneFloatFamily);
    _intrinsics["COS"] = DeclareTemplateFunction(L"cos", ReturnFirstArgType, OneFloatFamily);
    
    // RayQuery 控制方法
    _intrinsics["RAY_QUERY_PROCEED"] = DeclareTemplateMethod(nullptr, L"ray_query_proceed", BoolType, {});
    _intrinsics["RAY_QUERY_TERMINATE"] = DeclareTemplateMethod(nullptr, L"ray_query_terminate", VoidType, {});
    _intrinsics["RAY_QUERY_TRACE_RAY_INLINE"] = DeclareTemplateMethod(nullptr, L"ray_query_trace_ray_inline", VoidType, 
        { ASType, UIntType, UIntType, RayDescType });
    
    // RayQuery 状态查询
    _intrinsics["RAY_QUERY_COMMITTED_STATUS"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_status", UIntType, {});
    _intrinsics["RAY_QUERY_COMMITTED_RAY_T"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_ray_t", FloatType, {});
    _intrinsics["RAY_QUERY_WORLD_RAY_ORIGIN"] = DeclareTemplateMethod(nullptr, L"ray_query_world_ray_origin", Float3Type, {});
    _intrinsics["RAY_QUERY_WORLD_RAY_DIRECTION"] = DeclareTemplateMethod(nullptr, L"ray_query_world_ray_direction", Float3Type, {});
    
    // RayQuery 几何信息
    _intrinsics["RAY_QUERY_COMMITTED_TRIANGLE_BARYCENTRICS"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_triangle_barycentrics", Float2Type, {});
    _intrinsics["RAY_QUERY_COMMITTED_INSTANCE_INDEX"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_instance_index", UIntType, {});
    _intrinsics["RAY_QUERY_COMMITTED_GEOMETRY_INDEX"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_geometry_index", UIntType, {});
    _intrinsics["RAY_QUERY_COMMITTED_PRIMITIVE_INDEX"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_primitive_index", UIntType, {});
    
    // RayQuery 变换矩阵
    _intrinsics["RAY_QUERY_COMMITTED_OBJECT_TO_WORLD_3X4"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_object_to_world_3x4", Float3x4Type, {});
    _intrinsics["RAY_QUERY_COMMITTED_WORLD_TO_OBJECT_3X4"] = DeclareTemplateMethod(nullptr, L"ray_query_committed_world_to_object_3x4", Float3x4Type, {});
    
    // RayQuery 特殊查询（布尔表达式）
    _intrinsics["RAY_QUERY_IS_TRIANGLE_CANDIDATE"] = DeclareTemplateMethod(nullptr, L"ray_query_is_triangle_candidate", BoolType, {});
    _intrinsics["RAY_QUERY_IS_PROCEDURAL_CANDIDATE"] = DeclareTemplateMethod(nullptr, L"ray_query_is_procedural_candidate", BoolType, {});
}
```

### 类型系统层次结构

```cpp
// 基础声明类
class Decl {
public:
    virtual DeclKind kind() const = 0;
    const std::vector<Attr*>& attrs() const;
};

// 命名声明
class NamedDecl : public Decl {
    Name _name;
public:
    const Name& name() const;
};

// 类型声明层次
class TypeDecl : public NamedDecl {
public:
    virtual uint32_t size() const = 0;
    virtual uint32_t align() const = 0;
};

// 值类型（可以存储在变量中）
class ValueTypeDecl : public TypeDecl {
public:
    virtual bool is_scalar() const { return false; }
    virtual bool is_vector() const { return false; }
    virtual bool is_matrix() const { return false; }
    virtual bool is_structure() const { return false; }
};

// 资源类型（GPU 资源）
class ResourceTypeDecl : public TypeDecl {
public:
    virtual ResourceKind resource_kind() const = 0;
};
```

### 具体类型实现

#### 标量类型

```cpp
class ScalarTypeDecl : public ValueTypeDecl {
    ScalarKind _kind;
public:
    bool is_scalar() const override { return true; }
    ScalarKind scalar_kind() const { return _kind; }
    
    // GPU 内存布局
    uint32_t size() const override {
        switch (_kind) {
            case ScalarKind::Bool: return 4;    // GPU bool 是 4 字节
            case ScalarKind::Float: return 4;
            case ScalarKind::Double: return 8;
            case ScalarKind::Int: return 4;
            case ScalarKind::Uint: return 4;
            // ...
        }
    }
};
```

#### 向量类型

```cpp
class VectorTypeDecl : public ValueTypeDecl {
    const TypeDecl* _element;
    uint32_t _count;
public:
    bool is_vector() const override { return true; }
    const TypeDecl& element() const { return *_element; }
    uint32_t count() const { return _count; }
    
    // GPU 对齐规则
    uint32_t align() const override {
        // float3 需要 16 字节对齐
        if (_element->size() == 4 && _count == 3) return 16;
        return _element->size() * _count;
    }
};
```

#### 矩阵类型

```cpp
class MatrixTypeDecl : public ValueTypeDecl {
    const TypeDecl* _element;
    uint32_t _rows, _cols;
public:
    bool is_matrix() const override { return true; }
    
    // 列主序存储（GPU 标准）
    uint32_t size() const override {
        return _element->size() * _rows * _cols;
    }
    
    uint32_t align() const override {
        // 矩阵按列对齐
        return VectorTypeDecl(_element, _rows).align();
    }
};
```

#### 结构体类型

```cpp
class StructureTypeDecl : public ValueTypeDecl {
    std::vector<FieldDecl*> _fields;
    mutable uint32_t _size = 0;
    mutable uint32_t _align = 0;
public:
    bool is_structure() const override { return true; }
    
    // 计算 GPU 内存布局
    void calculate_layout() const {
        uint32_t offset = 0;
        for (auto field : _fields) {
            auto field_align = field->type().align();
            _align = std::max(_align, field_align);
            
            // 对齐到字段要求
            offset = (offset + field_align - 1) & ~(field_align - 1);
            field->set_offset(offset);
            offset += field->type().size();
        }
        
        // 结构体大小需要对齐
        _size = (offset + _align - 1) & ~(_align - 1);
    }
};
```

### 资源类型

#### Buffer 类型

```cpp
class BufferTypeDecl : public ResourceTypeDecl {
    const TypeDecl* _element;
    BufferFlags _flags;
public:
    ResourceKind resource_kind() const override { 
        return ResourceKind::Buffer; 
    }
    
    bool is_read_only() const { 
        return !(_flags & BufferFlags::ReadWrite); 
    }
    
    bool is_structured() const {
        return _element->is_structure();
    }
};

// 具体 Buffer 类型
class StructuredBufferTypeDecl : public BufferTypeDecl {};
class ByteAddressBufferTypeDecl : public BufferTypeDecl {};
class ConstantBufferTypeDecl : public BufferTypeDecl {};
```

#### 纹理类型

```cpp
class TextureTypeDecl : public ResourceTypeDecl {
    const TypeDecl* _element;
    TextureDimension _dimension;
    TextureFlags _flags;
public:
    ResourceKind resource_kind() const override { 
        return ResourceKind::Texture; 
    }
    
    bool is_array() const { 
        return _flags & TextureFlags::Array; 
    }
    
    bool is_ms() const { 
        return _flags & TextureFlags::Multisample; 
    }
    
    uint32_t dimension_count() const {
        switch (_dimension) {
            case TextureDimension::Tex1D: return 1;
            case TextureDimension::Tex2D: return 2;
            case TextureDimension::Tex3D: return 3;
            case TextureDimension::TexCube: return 2;
        }
    }
};
```

## 属性系统

### 属性基类

```cpp
class Attr {
public:
    virtual AttrKind kind() const = 0;
};

// 属性种类
enum class AttrKind {
    Stage,          // 着色器阶段
    Semantic,       // 语义绑定
    ResourceBind,   // 资源绑定
    KernelSize,     // 计算核大小
    Builtin,        // 内置变量
    // ...
};
```

### 具体属性实现

```cpp
// 着色器阶段属性
class StageAttr : public Attr {
    ShaderStage _stage;
public:
    AttrKind kind() const override { return AttrKind::Stage; }
    ShaderStage stage() const { return _stage; }
};

// 语义属性
class SemanticAttr : public Attr {
    SemanticType _semantic;
public:
    AttrKind kind() const override { return AttrKind::Semantic; }
    SemanticType semantic() const { return _semantic; }
};

// 资源绑定属性
class ResourceBindAttr : public Attr {
    uint32_t _group;    // Vulkan set / HLSL space
    uint32_t _binding;  // Vulkan binding / HLSL register
public:
    AttrKind kind() const override { return AttrKind::ResourceBind; }
    uint32_t group() const { return _group; }
    uint32_t binding() const { return _binding; }
};

// 计算核大小属性
class KernelSizeAttr : public Attr {
    uint32_t _x, _y, _z;
public:
    AttrKind kind() const override { return AttrKind::KernelSize; }
    uint32_t x() const { return _x; }
    uint32_t y() const { return _y; }
    uint32_t z() const { return _z; }
};
```

## 代码生成

### 代码生成器接口

```cpp
class ICodeGenerator {
public:
    virtual ~ICodeGenerator() = default;
    virtual String generate(const AST& ast) = 0;
    
protected:
    virtual void visitDecl(const Decl* decl) = 0;
    virtual void visitStmt(const Stmt* stmt) = 0;
    virtual void visitExpr(const Expr* expr) = 0;
    virtual void visitType(const TypeDecl* type) = 0;
};
```

### HLSL 生成器

HLSLGenerator 负责将 ShaderAST 转换为 HLSL 代码，特别是处理 RayQuery 方法的映射：

```cpp
// tools/LLVMTools/shader_compiler/AST/src/langs/HLSLGenerator.cpp (实际代码片段)
class HLSLGenerator : public ICodeGenerator {
    SourceBuilderNew sb;
    
public:
    String generate(const AST& ast) override {
        // 生成头部
        generate_header();
        
        // 访问所有声明
        for (auto decl : ast.declarations()) {
            visitDecl(decl);
        }
        
        return sb.to_string();
    }
    
protected:
    void generate_callexpr(const CallExpr* call) override {
        if (auto method = dyn_cast<MethodCallExpr>(call)) {
            auto object = method->object();
            auto type = object->type();
            
            // RayQuery 特殊处理
            if (auto as_ray_query = dynamic_cast<const RayQueryTypeDecl*>(type)) {
                visitExpr(object);
                sb.append(L".");
                
                auto method_name = method->name();
                
                // 方法名映射到 HLSL
                if (method_name == L"ray_query_proceed") sb.append(L"Proceed");
                else if (method_name == L"ray_query_terminate") sb.append(L"Abort");
                else if (method_name == L"ray_query_trace_ray_inline") sb.append(L"TraceRayInline");
                else if (method_name == L"ray_query_committed_status") sb.append(L"CommittedStatus");
                else if (method_name == L"ray_query_committed_ray_t") sb.append(L"CommittedRayT");
                else if (method_name == L"ray_query_world_ray_origin") sb.append(L"WorldRayOrigin");
                else if (method_name == L"ray_query_world_ray_direction") sb.append(L"WorldRayDirection");
                else if (method_name == L"ray_query_committed_triangle_barycentrics") sb.append(L"CommittedTriangleBarycentrics");
                else if (method_name == L"ray_query_committed_instance_index") sb.append(L"CommittedInstanceIndex");
                else if (method_name == L"ray_query_committed_geometry_index") sb.append(L"CommittedGeometryIndex");
                else if (method_name == L"ray_query_committed_primitive_index") sb.append(L"CommittedPrimitiveIndex");
                else if (method_name == L"ray_query_committed_object_to_world_3x4") sb.append(L"CommittedObjectToWorld3x4");
                else if (method_name == L"ray_query_committed_world_to_object_3x4") sb.append(L"CommittedWorldToObject3x4");
                
                // 特殊处理：生成比较表达式而非方法调用
                else if (method_name == L"ray_query_is_triangle_candidate") {
                    sb.clear_last(1); // 移除 "."
                    sb.append(L".CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE");
                    return; // 不需要参数
                }
                else if (method_name == L"ray_query_is_procedural_candidate") {
                    sb.clear_last(1); // 移除 "."
                    sb.append(L".CandidateType() == CANDIDATE_PROCEDURAL_PRIMITIVE");
                    return; // 不需要参数
                }
                
                // 生成参数
                sb.append(L"(");
                for (size_t i = 0; i < method->args().size(); ++i) {
                    if (i > 0) sb.append(L", ");
                    visitExpr(method->args()[i]);
                }
                sb.append(L")");
                return;
            }
        }
        
        // 默认处理
        // ... 其他表达式类型
    }
    
    void generate_resource_binding(const VarDecl* var) {
        if (auto bind = find_attr<ResourceBindAttr>(var)) {
            auto reg_char = get_register_char(var->type());
            sb.append_format(L" : register({}{}, space{})",
                           reg_char, bind->binding(), bind->group());
        }
    }
};
```

### MSL 生成器

```cpp
class MSLGenerator : public ICodeGenerator {
protected:
    void generate_function_signature(const FunctionDecl* func) {
        // Metal 特定的函数签名
        if (is_kernel_function(func)) {
            sb.append(L"kernel ");
        } else if (is_vertex_function(func)) {
            sb.append(L"vertex ");
        } else if (is_fragment_function(func)) {
            sb.append(L"fragment ");
        }
        
        // 返回类型和函数名
        visitType(func->return_type());
        sb.append(L" ");
        sb.append(func->name());
        
        // 参数列表（包含 Metal 属性）
        generate_parameters_with_attributes(func);
    }
    
    String get_metal_attribute(const Attr* attr) {
        static const std::unordered_map<String, String> AttrMap = {
            { L"VertexID", L"[[vertex_id]]" },
            { L"InstanceID", L"[[instance_id]]" },
            { L"Position", L"[[position]]" },
            { L"ThreadID", L"[[thread_position_in_grid]]" },
            { L"GroupID", L"[[threadgroup_position_in_grid]]" },
        };
        // ...
    }
};
```

## 实际使用示例

### RayQuery 着色器示例

基于实际的光线追踪计算着色器，展示从 HLSL 到 CppSL 的转换：

```cpp
// engine/tools/LLVMTools/shader_compiler/ShaderSTL/raytracing_sample.cpp
#include "std/std.hpp"

#define WIDTH 3200
#define HEIGHT 2400

// 外部资源声明
[[binding(0, 0)]]
extern rw_structured_buffer<float4> buf;

[[binding(0, 1)]] 
extern acceleration_structure AS;

// 光线追踪函数
float4 trace(uint2 tid, uint2 tsize) {
    ray_desc ray;
    ray.origin = float3(float(tid.x) / float(tsize.x), float(tid.y) / float(tsize.y), 100.0f);
    ray.direction = float3(0, 0, -1.0f);
    ray.t_min = 0.01f;
    ray.t_max = 9999.0f;
    
    // 创建 RayQuery 对象
    ray_query<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.trace_ray_inline(AS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.proceed();
    
    // 检查击中状态
    if (q.committed_status() == COMMITTED_TRIANGLE_HIT) {
        return float4(q.committed_triangle_barycentrics(), 1, 1);
    } else {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

// 计算着色器入口点
[[numthreads(32, 32, 1)]]
void compute_main([[sv_thread_id]] uint3 tid) {
    uint2 tsize = uint2(WIDTH, HEIGHT);
    uint row_pitch = tsize.x;
    buf[tid.x + (tid.y * row_pitch)] = trace(tid.xy, tsize);
}
```

这个示例展示了：
1. **资源绑定**：使用 `[[binding(register, space)]]` 绑定缓冲区和加速结构
2. **RayQuery 使用**：创建 ray_query 对象并调用其方法
3. **方法映射**：`q.proceed()` → `q.Proceed()`，`q.committed_status()` → `q.CommittedStatus()`
4. **入口点声明**：使用 `[[compute_shader("name")]]` 和 `[[kernel_3d(x, y, z)]]` 声明计算着色器
5. **内置变量**：使用 `[[sv_thread_id]]` 标记线程ID参数

### 生成的 HLSL 代码

上述 CppSL 代码会生成如下 HLSL：

```hlsl
#define WIDTH 3200
#define HEIGHT 2400

RWStructuredBuffer<float4> buf : register(u0, space0);
RaytracingAccelerationStructure AS : register(t0, space0);

float4 trace(uint2 tid, uint2 tsize) {
    RayDesc ray;
    ray.Origin = float3((float)tid.x / (float)tsize.x, (float)tid.y / (float)tsize.y, 100.0f);
    ray.Direction = float3(0, 0, -1.0f);
    ray.TMin = 0.01f;
    ray.TMax = 9999.0f;
    
    RayQuery<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.TraceRayInline(AS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.Proceed();
    
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        return float4(q.CommittedTriangleBarycentrics(), 1, 1);
    } else {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

[numthreads(32, 32, 1)]
void compute_main(uint3 tid : SV_DispatchThreadID) {
    uint2 tsize = uint2(WIDTH, HEIGHT);
    uint row_pitch = tsize.x;
    buf[tid.x + (tid.y * row_pitch)] = trace(tid.xy, tsize);
}
```

### 构建简单的计算着色器 AST

```cpp
// 创建 AST
AST ast;

// 声明外部 Buffer
auto buffer_type = ast.StructuredBufferType(ast.Float4Type, BufferFlags::ReadWrite);
auto buffer_var = ast.DeclareVariable(L"output_buffer", buffer_type);
buffer_var->add_attr(new ResourceBindAttr(0, 0));

// 创建计算函数
auto tid_param = ast.DeclareParameter(L"tid", ast.Uint2Type);
tid_param->add_attr(new BuiltinAttr(L"ThreadID"));

auto func_body = ast.Block({
    // uint index = tid.x + tid.y * 1920;
    auto index_var = ast.DeclareVariable(L"index", ast.UintType,
        ast.Binary(BinaryOp::ADD,
            ast.Member(ast.Ref(tid_param), L"x"),
            ast.Binary(BinaryOp::MUL,
                ast.Member(ast.Ref(tid_param), L"y"),
                ast.Literal(1920u))));
    
    // output_buffer[index] = float4(1, 0, 0, 1);
    ast.ExprStmt(
        ast.ArrayAssign(
            ast.Ref(buffer_var),
            ast.Ref(index_var),
            ast.Construct(ast.Float4Type, {
                ast.Literal(1.0f), ast.Literal(0.0f),
                ast.Literal(0.0f), ast.Literal(1.0f)
            })));
});

auto compute_func = ast.DeclareFunction(L"compute_main", ast.VoidType, 
                                       {tid_param}, func_body);
compute_func->add_attr(new StageAttr(ShaderStage::Compute));
compute_func->add_attr(new KernelSizeAttr(8, 8, 1));
```

### 生成 HLSL 代码

```cpp
HLSLGenerator hlsl_gen;
String hlsl_code = hlsl_gen.generate(ast);

// 输出：
// RWStructuredBuffer<float4> output_buffer : register(u0, space0);
// 
// [numthreads(8, 8, 1)]
// void compute_main(uint2 tid : SV_DispatchThreadID) {
//     uint index = tid.x + tid.y * 1920;
//     output_buffer[index] = float4(1, 0, 0, 1);
// }
```

## GPU 内存布局规则

### 标准布局规则 (std140/std430)

```cpp
// GPU 对齐计算
template <typename T, int N>
inline static constexpr int gpu_vec_align() {
    // 标量和 vec2 按自身大小对齐
    if constexpr (N <= 2) return sizeof(T) * N;
    // vec3 和 vec4 按 16 字节对齐
    else return 16;
}

// 结构体成员对齐
struct GPULayout {
    static uint32_t align_offset(uint32_t offset, uint32_t alignment) {
        return (offset + alignment - 1) & ~(alignment - 1);
    }
    
    static uint32_t struct_size(uint32_t last_offset, uint32_t last_size, uint32_t alignment) {
        uint32_t size = last_offset + last_size;
        return align_offset(size, alignment);
    }
};
```

## 优化和验证

### AST 优化

```cpp
class ASTOptimizer {
public:
    void optimize(AST& ast) {
        // 常量折叠
        constant_folding(ast);
        
        // 死代码消除
        dead_code_elimination(ast);
        
        // 公共子表达式消除
        common_subexpression_elimination(ast);
    }
    
private:
    void constant_folding(AST& ast) {
        // 折叠常量表达式
        // 2 + 3 -> 5
        // float3(1, 2, 3).x -> 1
    }
};
```

### AST 验证

```cpp
class ASTValidator {
public:
    bool validate(const AST& ast, std::vector<Error>& errors) {
        // 类型检查
        check_types(ast, errors);
        
        // 资源绑定冲突检查
        check_resource_bindings(ast, errors);
        
        // 语义检查
        check_semantics(ast, errors);
        
        return errors.empty();
    }
};
```

## 扩展性设计

### 添加新的类型

```cpp
// 添加半精度浮点支持
class HalfTypeDecl : public ScalarTypeDecl {
public:
    uint32_t size() const override { return 2; }
    uint32_t align() const override { return 2; }
};

// 添加到 AST
ast.HalfType = new HalfTypeDecl();
ast.Half2Type = ast.VectorType(ast.HalfType, 2);
ast.Half3Type = ast.VectorType(ast.HalfType, 3);
ast.Half4Type = ast.VectorType(ast.HalfType, 4);
```

### 添加新的目标语言

```cpp
// GLSL 生成器
class GLSLGenerator : public ICodeGenerator {
    // 实现 GLSL 特定的代码生成逻辑
};

// SPIR-V 生成器
class SPIRVGenerator : public ICodeGenerator {
    // 生成 SPIR-V 字节码
};
```

## 最佳实践

### 设计原则

1. **保持简单**：只添加着色器真正需要的特性
2. **类型安全**：在 AST 层面保证类型正确性
3. **平台无关**：避免在 AST 中引入平台特定概念
4. **可验证性**：设计时考虑验证和优化需求

### 性能考虑

1. **内存池**：使用内存池分配 AST 节点
2. **引用语义**：使用指针避免深拷贝
3. **延迟计算**：布局信息等按需计算
4. **缓存友好**：相关节点在内存中连续存储

## 总结

ShaderAST 是 SakuraEngine 着色器系统的核心，它提供了：

1. **清晰的抽象**：简化的 AST 结构易于理解和操作
2. **强大的类型系统**：GPU 友好的类型表示和布局
3. **灵活的属性机制**：支持各种着色器元数据，特别是 `callop` 属性系统
4. **高效的代码生成**：针对不同目标优化的生成器，支持复杂的方法映射
5. **良好的扩展性**：易于添加新特性和目标平台

### RayQuery 实现的核心经验

通过 RayQuery 功能的完整实现，我们验证了系统设计的有效性：

1. **内置函数注册**：在 AST 构造时统一注册所有 25+ 个 RayQuery 方法
2. **属性驱动映射**：`[[callop("NAME")]]` 属性提供了清晰的 C++ 到 Shader 映射机制
3. **代码生成灵活性**：HLSLGenerator 能够处理方法调用映射、特殊表达式生成等复杂需求
4. **类型安全保证**：强类型的内置函数声明确保编译期错误检查
5. **跨平台兼容**：通过抽象层隔离平台差异，保证核心逻辑的一致性

### 实际工程价值

这个设计不仅是理论上的抽象，更是经过实际工程验证的解决方案：

- **开发效率**：开发者可以使用熟悉的 C++ 语法编写着色器
- **类型安全**：编译期捕获大部分错误，减少运行时调试
- **工具链复用**：充分利用现有的 C++ 开发工具和基础设施
- **维护性**：清晰的代码结构和文档化的映射关系便于维护
- **扩展性**：添加新的 GPU 特性变得简单和标准化

通过这个设计，SakuraEngine 实现了用 C++ 编写着色器的愿景，同时保持了高性能和跨平台的特性。RayQuery 的成功实现证明了这个架构的实用性和可扩展性。