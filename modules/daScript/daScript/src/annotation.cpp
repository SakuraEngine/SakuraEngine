#include "SkrDAScript/annotation.hpp"
#include "types.hpp"
#include "daScript/simulate/simulate_visit_op.h"
#include "daScript/simulate/simulate_nodes.h"

namespace 
{
using namespace ::das;

struct SimNode_Allocate : ::das::SimNode {
    DAS_PTR_NODE;
    SimNode_Allocate(const ::das::LineInfo& a, uint64_t sz) : SimNode(a), sz(sz) {}
    __forceinline char* compute (::das::Context & context) 
    {
        DAS_PROFILE_NODE
        auto res = sakura_malloc(sz);
#if DAS_ENABLE_SMART_PTR_TRACKING
        Context::sptrAllocations.push_back(res);
#endif
        return (char*) res;
    }

    virtual ::das::SimNode* visit(::das::SimVisitor& vis) override
    {
        V_BEGIN();
        vis.op("NewHandle", sizeof(uint32_t), ::das::typeName<uint32_t>::name());
        V_END();
    }

    uint64_t sz;
};

struct SimNode_Deallocate : ::das::SimNode_Delete {
    SimNode_Deallocate (const LineInfo& a, SimNode* s, uint32_t t) : SimNode_Delete(a,s,t) {}
    virtual vec4f DAS_EVAL_ABI eval ( Context & context ) override {
        DAS_PROFILE_NODE
        auto pH = (void**)subexpr->evalPtr(context);
        for ( uint32_t i=0, is=total; i!=is; ++i, pH++ ) {
            if (*pH) 
            {
                sakura_free(*pH);
                *pH = nullptr;
            }
        }
        return v_zero();
    }

    virtual ::das::SimNode* visit(::das::SimVisitor& vis) override
    {
        V_BEGIN();
        vis.op("DeleteHandlePtr", sizeof(uint32_t), ::das::typeName<uint32_t>::name());
        V_SUB(subexpr);
        V_ARG(total);
        V_END();
    }

    uint64_t sz;
};
}

#include "daScript/simulate/simulate_visit_op_undef.h"

namespace skr {
namespace das {

Annotation::~Annotation() SKR_NOEXCEPT {}
TypeAnnotation::~TypeAnnotation() SKR_NOEXCEPT {}

StructureAnnotation* StructureAnnotation::Create(const char8_t* name, const char8_t* cppname, Library* library) SKR_NOEXCEPT
{
    return SkrNew<StructureAnnotationImpl>(name, cppname, library);
}

void StructureAnnotation::Free(StructureAnnotation* annotation) SKR_NOEXCEPT
{
    SkrDelete(annotation);
}

StructureAnnotation::~StructureAnnotation() SKR_NOEXCEPT { }


StructureAnnotationImpl::StructureAnnotationImpl(
    const char8_t* name, const char8_t* cppname, Library* library) SKR_NOEXCEPT
    : Lib(static_cast<LibraryImpl*>(library)), 
      annotation(::das::make_smart<Structure>(
        (const char*)name, (const char*)cppname, 
        &static_cast<LibraryImpl*>(library)->libGroup)
    )
{

}

void StructureAnnotationImpl::add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, TypeDecl* typedecl) SKR_NOEXCEPT
{
    auto Decl = static_cast<TypeDeclImpl*>(typedecl);
    annotation->addFieldEx((const char*)na, (const char*)cppna, offset, Decl->decl);
}

void StructureAnnotationImpl::add_field(const char8_t* na, const char8_t* cppna, uint32_t offset, EBuiltinType type) SKR_NOEXCEPT
{
    ::das::TypeDeclPtr t = nullptr;
    switch (type)
    {
    case EBuiltinType::BITFIELD: t = ::das::makeType<::das::Bitfield>(Lib->libGroup); break;
    case EBuiltinType::UINT8: t = ::das::makeType<uint8_t>(Lib->libGroup); break;
    case EBuiltinType::INT8: t = ::das::makeType<int8_t>(Lib->libGroup); break;
    case EBuiltinType::UINT16: t = ::das::makeType<uint16_t>(Lib->libGroup); break;
    case EBuiltinType::INT16: t = ::das::makeType<int16_t>(Lib->libGroup); break;
    case EBuiltinType::UINT32: t = ::das::makeType<uint32_t>(Lib->libGroup); break;
    case EBuiltinType::INT32: t = ::das::makeType<int32_t>(Lib->libGroup); break;
    case EBuiltinType::UINT64: t = ::das::makeType<uint64_t>(Lib->libGroup); break;
    case EBuiltinType::INT64: t = ::das::makeType<int64_t>(Lib->libGroup); break;
    case EBuiltinType::FLOAT: t = ::das::makeType<float>(Lib->libGroup); break;
    case EBuiltinType::DOUBLE: t = ::das::makeType<double>(Lib->libGroup); break;
    case EBuiltinType::VOID: t = ::das::makeType<void>(Lib->libGroup); break;
    case EBuiltinType::PTR: t = ::das::makeType<void*>(Lib->libGroup); break;
    case EBuiltinType::ENUMERATION: t = ::das::makeType<::das::EnumStub>(Lib->libGroup); break;
    case EBuiltinType::ENUMERATION8: t = ::das::makeType<::das::EnumStub8>(Lib->libGroup); break;
    case EBuiltinType::ENUMERATION16: t = ::das::makeType<::das::EnumStub16>(Lib->libGroup); break;
    case EBuiltinType::FLOAT2: t = ::das::makeType<::das::float2>(Lib->libGroup); break;
    case EBuiltinType::FLOAT3: t = ::das::makeType<::das::float3>(Lib->libGroup); break;
    case EBuiltinType::FLOAT4: t = ::das::makeType<::das::float4>(Lib->libGroup); break;
    case EBuiltinType::UINT2: t = ::das::makeType<::das::uint2>(Lib->libGroup); break;
    case EBuiltinType::INT2: t = ::das::makeType<::das::int2>(Lib->libGroup); break;
    case EBuiltinType::UINT3: t = ::das::makeType<::das::uint3>(Lib->libGroup); break;
    case EBuiltinType::INT3: t = ::das::makeType<::das::int3>(Lib->libGroup); break;
    case EBuiltinType::UINT4: t = ::das::makeType<::das::uint4>(Lib->libGroup); break;
    case EBuiltinType::INT4: t = ::das::makeType<::das::int4>(Lib->libGroup); break;
    case EBuiltinType::URANGE: t = ::das::makeType<::das::urange>(Lib->libGroup); break;
    case EBuiltinType::RANGE: t = ::das::makeType<::das::range>(Lib->libGroup); break;
    case EBuiltinType::URANGE64: t = ::das::makeType<::das::urange64>(Lib->libGroup); break;
    case EBuiltinType::RANGE64: t = ::das::makeType<::das::range64>(Lib->libGroup); break;
    case EBuiltinType::ARRAY: t = ::das::makeType<::das::Array>(Lib->libGroup); break;
    case EBuiltinType::TABLE: t = ::das::makeType<::das::Table>(Lib->libGroup); break;
    case EBuiltinType::BLOCK: t = ::das::makeType<::das::Block>(Lib->libGroup); break;
    case EBuiltinType::FUNCTION: t = ::das::makeType<::das::Func>(Lib->libGroup); break;
    case EBuiltinType::LAMBDA: t = ::das::makeType<::das::Lambda>(Lib->libGroup); break;
    case EBuiltinType::TUPLE: t = ::das::makeType<::das::Tuple>(Lib->libGroup); break;
    case EBuiltinType::VARIANT: t = ::das::makeType<::das::Variant>(Lib->libGroup); break;
    default: break;
    }
    annotation->addFieldEx((const char*)na, (const char*)cppna, offset, t);
}

::das::SimNode * StructureAnnotationImpl::Structure::simulateGetNew(
    ::das::Context& context, const ::das::LineInfo& at) const
{
    return context.code->makeNode<::das::SimNode_NewHandle<uint32_t, false>>(at);
}

::das::SimNode* StructureAnnotationImpl::Structure::simulateDeletePtr(
    ::das::Context& context, const ::das::LineInfo& at, ::das::SimNode* sube, uint32_t count) const
{
    return context.code->makeNode<::das::SimNode_DeleteHandlePtr<uint32_t, false>>(at,sube,count);
}

/*
::das::SimNode * StructureAnnotationImpl::Structure::simulateGetNew(
    ::das::Context& context, const ::das::LineInfo& at) const
{
    return context.code->makeNode<::SimNode_Allocate>(at, sizeof(uint32_t));
}

::das::SimNode* StructureAnnotationImpl::Structure::simulateDeletePtr(
    ::das::Context& context, const ::das::LineInfo& at, ::das::SimNode* sube, uint32_t count) const
{
    return context.code->makeNode<::SimNode_Deallocate>(at,sube,count);
}
*/

} // namespace das
} // namespace skr