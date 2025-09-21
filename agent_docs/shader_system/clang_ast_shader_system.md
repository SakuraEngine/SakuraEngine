# SakuraEngine Clang AST 着色器系统设计文档

## 概述

SakuraEngine 采用了创新的着色器开发方式，使用 **C++ 作为着色器语言**，通过 Clang LibTooling 解析 C++ AST，再转换为中间表示（ShaderAST），最终生成 HLSL/MSL 等目标着色器代码。

## 系统架构

```
C++ 源码 → Clang AST → Shader AST → HLSL/MSL/GLSL
```

### 核心组件

- **CppSLLLVM**：基于 Clang LibTooling 的 C++ AST 解析器（位于 `tools/shader_compiler/LLVM`）
- **CppSLAst**：中间层着色器 AST 表示（位于 `tools/shader_compiler/AST`）
- **代码生成器**：将 Shader AST 转换为各种目标语言

## Clang AST 解析实现

### ShaderCompiler 主框架

```cpp
// tools/shader_compiler/LLVM/src/shader_compiler.hpp
struct ShaderCompiler {
    static ShaderCompiler* Create(int argc, const char **argv);
    virtual int Run() = 0;
};

// 实现类使用 Clang Tooling
struct ShaderCompilerImpl : public ShaderCompiler {
    CommonOptionsParser OptionsParser;
    ClangTool tool;
    skr::CppSL::AST AST;
    
    int Run() override {
        // 创建 AST Consumer 并运行
        tool.run(newFrontendActionFactory<ASTFrontendAction>().get());
        return 0;
    }
};
```

### AST Consumer 设计

AST Consumer 负责遍历 Clang AST 并构建 ShaderAST：

```cpp
class ASTConsumer : public clang::ASTConsumer, 
                    public clang::RecursiveASTVisitor<ASTConsumer> {
public:
    // 访问器方法
    bool VisitFunctionDecl(const clang::FunctionDecl* x);
    bool VisitRecordDecl(const clang::RecordDecl* x);
    bool VisitEnumDecl(const clang::EnumDecl* x);
    bool VisitVarDecl(const clang::VarDecl* x);
    
private:
    // 类型映射表
    std::map<const clang::TagDecl*, skr::CppSL::TypeDecl*> _tag_types;
    std::map<const clang::BuiltinType::Kind, skr::CppSL::TypeDecl*> _builtin_types;
    std::map<const clang::FunctionDecl*, skr::CppSL::FunctionDecl*> _funcs;
    
    // 转换方法
    TypeDecl* TranslateType(const clang::QualType& type);
    Stmt* TranslateStmt(const clang::Stmt* stmt);
    Expr* TranslateExpr(const clang::Expr* expr);
};
```

## C++ 到 ShaderAST 的转换

### 类型系统映射

#### 基础类型映射

```cpp
void ASTConsumer::SetupBuiltinTypes() {
    // 标量类型
    addType(Context.FloatTy, AST.FloatType);
    addType(Context.IntTy, AST.IntType);
    addType(Context.BoolTy, AST.BoolType);
    addType(Context.VoidTy, AST.VoidType);
    
    // 无符号类型
    addType(Context.UnsignedIntTy, AST.UintType);
    addType(Context.UnsignedShortTy, AST.UshortType);
}
```

#### 向量类型识别

通过模板特化识别向量类型：

```cpp
if (TSD && What == "vec") {
    const auto ET = Arguments.get(0).getAsType();  // 元素类型
    const uint64_t N = Arguments.get(1).getAsIntegral(); // 维度
    
    if (getType(ET) == AST.FloatType) {
        const TypeDecl* Types[] = { 
            AST.Float2Type, AST.Float3Type, AST.Float4Type 
        };
        addType(ThisQualType, Types[N - 2]);
    }
    else if (getType(ET) == AST.IntType) {
        const TypeDecl* Types[] = { 
            AST.Int2Type, AST.Int3Type, AST.Int4Type 
        };
        addType(ThisQualType, Types[N - 2]);
    }
}
```

### 属性系统

SakuraEngine 着色器系统使用 Clang 注解属性进行元数据标记：

#### 1. 预定义属性宏（推荐使用）

使用 `attributes.hpp` 中定义的宏：

```cpp
#include "std/attributes.hpp"

// 计算着色器入口点
[[compute_shader("compute_main")]]
[[kernel_3d(32, 32, 1)]]
void compute_main([[sv_thread_id]] uint3 tid);

// 顶点着色器
[[vertex_shader("vs_main")]]
PSOutput vs_main(VSInput input);

// 片段着色器
[[fragment_shader("ps_main")]]
float4 ps_main(PSInput input);

// 内置变量
[[sv_thread_id]] uint3 tid;
[[sv_position]] float4 position;
[[sv_vertex_id]] uint vid;
```

#### 2. 资源绑定属性

使用绑定宏：

```cpp
// 资源绑定：binding(register, space)
[[binding(0, 0)]]
extern rw_structured_buffer<float4> output_buffer;

[[binding(0, 1)]]
extern acceleration_structure AS;
```

#### 3. callop 属性（内置函数映射）

用于将 C++ 方法映射到着色器内置函数：

```cpp
template <uint32 flags>
struct ray_query {
    [[callop("RAY_QUERY_PROCEED")]] bool proceed();
    [[callop("RAY_QUERY_COMMITTED_STATUS")]] uint32 committed_status();
    [[callop("RAY_QUERY_WORLD_RAY_ORIGIN")]] float3 world_ray_origin();
    // ... 更多方法
};
```

#### 4. 原始 clang::annotate 属性

```cpp
// 完整的 clang::annotate 语法
[[clang::annotate("skr-shader", "stage", "compute", "compute_main")]]
[[clang::annotate("skr-shader", "kernel", 16, 16, 1)]]
void compute_main([[clang::annotate("skr-shader", "builtin", "ThreadID")]] uint3 tid);

[[clang::annotate("skr-shader", "stage", "vertex", "vs_main")]]
PSOutput vs_main(VSInput input);

[[clang::annotate("skr-shader", "binding", 0, 0)]]
Buffer<float4>& buffer;
```

### 语句和表达式转换

#### 二元运算符处理

```cpp
Stmt* ASTConsumer::TranslateStmt(const clang::Stmt* x) {
    if (auto binOp = dyn_cast<BinaryOperator>(x)) {
        auto op = TranslateBinaryOp(binOp->getOpcode());
        auto left = TranslateStmt(binOp->getLHS());
        auto right = TranslateStmt(binOp->getRHS());
        return AST.Binary(op, left, right);
    }
}

BinaryOp TranslateBinaryOp(clang::BinaryOperatorKind op) {
    switch (op) {
        case BO_Add: return BinaryOp::ADD;
        case BO_Sub: return BinaryOp::SUB;
        case BO_Mul: return BinaryOp::MUL;
        case BO_Div: return BinaryOp::DIV;
        // ... 其他运算符
    }
}
```

#### 函数调用处理

```cpp
if (auto call = dyn_cast<CallExpr>(x)) {
    auto callee = TranslateStmt(call->getCallee());
    std::vector<Expr*> args;
    for (auto arg : call->arguments()) {
        args.push_back(TranslateStmt(arg));
    }
    return AST.CallFunction(callee, args);
}
```

#### 控制流语句

```cpp
// If 语句
if (auto ifStmt = dyn_cast<IfStmt>(x)) {
    auto cond = TranslateExpr(ifStmt->getCond());
    auto then = TranslateStmt(ifStmt->getThen());
    auto else_ = ifStmt->getElse() ? TranslateStmt(ifStmt->getElse()) : nullptr;
    return AST.If(cond, then, else_);
}

// For 循环
if (auto forStmt = dyn_cast<ForStmt>(x)) {
    auto init = TranslateStmt(forStmt->getInit());
    auto cond = TranslateExpr(forStmt->getCond());
    auto inc = TranslateStmt(forStmt->getInc());
    auto body = TranslateStmt(forStmt->getBody());
    return AST.For(init, cond, inc, body);
}
```

### 特殊着色器构造处理

#### 向量构造

```cpp
// 识别向量构造函数
if (isVectorConstruct(call)) {
    auto type = getVectorType(call);
    std::vector<Expr*> components;
    for (auto arg : call->arguments()) {
        components.push_back(TranslateExpr(arg));
    }
    return AST.Construct(type, components);
}
```

#### 纹理采样

```cpp
// 纹理采样调用
if (isSampleCall(call)) {
    auto texture = TranslateExpr(call->getArg(0));
    auto sampler = TranslateExpr(call->getArg(1));
    auto coords = TranslateExpr(call->getArg(2));
    return AST.SampleTexture(texture, sampler, coords);
}
```

## 使用示例

### RayQuery 光线追踪示例

基于实际项目的光线追踪计算着色器：

```cpp
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
    
    // 创建 RayQuery 对象并执行光线追踪
    ray_query<RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    q.trace_ray_inline(AS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, ray);
    q.proceed();
    
    // 检查击中状态并返回结果
    if (q.committed_status() == COMMITTED_TRIANGLE_HIT) {
        return float4(q.committed_triangle_barycentrics(), 1, 1);
    } else {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
}

// 计算着色器入口点
[[compute_shader("compute_main")]]
[[kernel_3d(32, 32, 1)]]
void compute_main([[sv_thread_id]] uint3 tid) {
    uint2 tsize = uint2(WIDTH, HEIGHT);
    uint row_pitch = tsize.x;
    buf[tid.x + (tid.y * row_pitch)] = trace(tid.xy, tsize);
}
```

### 计算着色器示例

```cpp
#include "std/std.hpp"

// 声明外部资源
[[binding(0, 0)]]
extern rw_structured_buffer<float4> output_buffer;

[[binding(1, 0)]]
extern texture_2d<float4> input_texture;

// 辅助函数
float4 process_pixel(uint2 coord, uint2 size) {
    float2 uv = float2(coord) / float2(size);
    return float4(uv.x, uv.y, 0.0f, 1.0f);
}

// 计算着色器入口点
[[compute_shader("compute_main")]]
[[kernel_3d(8, 8, 1)]]
void compute_main([[sv_thread_id]] uint3 tid) {
    const uint2 size = uint2(1920, 1080);
    
    if (tid.x < size.x && tid.y < size.y) {
        uint index = tid.x + tid.y * size.x;
        output_buffer[index] = process_pixel(tid.xy, size);
    }
}
```

### 顶点/像素着色器示例

```cpp

// 输入输出结构
struct VSInput {
    [[clang::annotate("skr-shader", "semantic", "POSITION")]]
    float3 position;
    
    [[clang::annotate("skr-shader", "semantic", "TEXCOORD0")]]
    float2 uv;
};

struct PSInput {
    [[stage_inout]]
    [[sv_position]]
    float4 position;
    
    float2 uv;
};

// 顶点着色器
[[vertex_shader("vs_main")]]
PSInput vs_main(VSInput input) {
    PSInput output;
    output.position = float4(input.position, 1.0f);
    output.uv = input.uv;
    return output;
}

// 像素着色器
[[fragment_shader("ps_main")]]
float4 ps_main(PSInput input) {
    return float4(input.uv.x, input.uv.y, 0.0f, 1.0f);
}
```

## 技术优势

1. **类型安全**：利用 C++ 的强类型系统在编译期捕获错误
2. **代码复用**：可以使用 C++ 的函数、模板等特性复用代码
3. **工具链支持**：可以使用现有的 C++ IDE、调试器等工具
4. **渐进式迁移**：可以逐步将传统着色器代码迁移到 C++
5. **跨平台一致性**：同一份代码生成不同平台的着色器

## 限制和注意事项

1. **C++ 子集**：只支持 C++ 的一个子集，不支持动态内存分配、虚函数等
2. **GPU 限制**：需要遵守 GPU 编程的限制（如递归深度、动态分支等）
3. **平台差异**：某些特性在不同平台可能有细微差异
4. **调试支持**：虽然可以使用 C++ 工具，但 GPU 调试仍需专门工具

## 总结

SakuraEngine 的 Clang AST 着色器系统展示了如何利用现有的编译器基础设施来构建领域特定语言。通过将 C++ 作为着色器语言，不仅提供了类型安全和代码复用的优势，还大大简化了跨平台着色器开发的复杂性。

### 实际工程验证

通过 RayQuery 功能的完整实现，系统经受了实际工程的检验：

1. **属性系统的实用性**：`sattr` 和 `callop` 属性提供了简洁而强大的元数据标记机制
2. **类型映射的准确性**：从 C++ 类型到 GPU 类型的映射保持了语义一致性
3. **代码生成的灵活性**：能够处理复杂的方法映射和特殊表达式生成
4. **扩展性的验证**：新增 25+ 个 RayQuery 内置函数没有对现有架构造成冲击

### 核心价值

- **开发体验**：开发者可以使用熟悉的 C++ 语法和 IDE 进行着色器开发
- **类型安全**：编译期类型检查大大减少了运行时错误
- **维护性**：清晰的代码结构和映射关系便于长期维护
- **性能**：生成的着色器代码与手写 HLSL 性能相当
- **跨平台**：单一代码库支持多种着色器语言输出

这是一个优雅且实用的工程解决方案，为现代游戏引擎的着色器开发提供了新的可能性。