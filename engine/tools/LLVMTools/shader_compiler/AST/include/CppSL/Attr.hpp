#pragma once
#include "Stmt.hpp"

namespace skr::CppSL {

struct Attr
{
    virtual ~Attr() = default;
};

struct AlignAttr final : public Attr
{
public:
    uint32_t alignment() const { return _alignment; }

private:
    friend struct AST;
    AlignAttr(uint32_t alignment);
    uint32_t _alignment = 0;
};

struct BuiltinAttr : public Attr
{
public:
    const String& name() const { return _name; }

private:
    friend struct AST;
    BuiltinAttr(const String& name);
    String _name;
};

struct SemanticAttr : public Attr
{
public:
    SemanticType semantic() const { return _semantic; }
    static bool GetSemanticQualifier(SemanticType semantic, ShaderStage stage, EVariableQualifier& out);

protected:
    friend struct AST;
    SemanticAttr(SemanticType semantic);
    SemanticType _semantic;
};

struct InterpolationAttr : public Attr
{
public:
    InterpolationMode mode() const { return _mode; }

protected:
    friend struct AST;
    InterpolationAttr(InterpolationMode mode);
    InterpolationMode _mode;
};

struct KernelSizeAttr : public Attr
{
public:
    uint32_t x() const { return _x; }
    uint32_t y() const { return _y; }
    uint32_t z() const { return _z; }

private:
    friend struct AST;
    KernelSizeAttr(uint32_t x, uint32_t y, uint32_t z);
    uint32_t _x = 1;
    uint32_t _y = 1;
    uint32_t _z = 1;
};

struct ResourceBindAttr : public Attr
{
public:
    uint32_t group() const { return _group; }
    uint32_t binding() const { return _binding; }

private:
    friend struct AST;
    ResourceBindAttr();
    ResourceBindAttr(uint32_t group, uint32_t binding);
    uint32_t _group = ~0;
    uint32_t _binding = ~0;
};

struct PushConstantAttr : public Attr
{
private:
    friend struct AST;
    PushConstantAttr();
};

struct NormFloatAccessAttr : public Attr
{
public:
    bool is_signed() const { return _is_signed; }

private:
    friend struct AST;
    NormFloatAccessAttr(bool is_signed);
    bool _is_signed = false;
};

struct GloballyCoherentAttr : public Attr
{
private:
    friend struct AST;
    GloballyCoherentAttr();
};

struct StageInoutAttr : public Attr
{
private:
    friend struct AST;
    StageInoutAttr();
};

struct StageAttr : public Attr
{
public:
    ShaderStage stage() const { return _stage; }

private:
    friend struct AST;
    StageAttr(ShaderStage stage);
    ShaderStage _stage;
};

struct BranchAttr : public Attr
{
private:
    friend struct AST;
    BranchAttr();
};

struct FlattenAttr : public Attr
{
private:
    friend struct AST;
    FlattenAttr();
};

struct LoopAttr : public Attr
{
private:
    friend struct AST;
    LoopAttr();
};

struct UnrollAttr : public Attr
{
public:
    uint32_t count() const { return _N; }
    
private:
    friend struct AST;
    UnrollAttr(uint32_t N = UINT32_MAX);
    uint32_t _N = UINT32_MAX;
};


} // namespace skr::CppSL