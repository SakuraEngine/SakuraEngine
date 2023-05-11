#include "types.hpp"
#include "daScript/simulate/interop.h"

namespace 
{
using namespace ::das;

struct SimNode_ExternCall : public ::das::SimNode_ExtFuncCallBase
{
    SimNode_ExternCall(skr::das::BuiltInSimProc call, const ::das::LineInfo & at, const char * fnName)
        : SimNode_ExtFuncCallBase(at,fnName), call(call) { }
    virtual vec4f DAS_EVAL_ABI eval(::das::Context& context) override 
    {
        DAS_PROFILE_NODE
        if (cmresEval)
        {
            void * cmres = cmresEval->evalPtr(context);
            return call(arguments, context, cmres);
        }
        return call(arguments, context, nullptr);
    }
    skr::das::BuiltInSimProc call = nullptr;
    /*
#define EVAL_NODE(TYPE,CTYPE)\
    virtual CTYPE eval##TYPE ( Context & context ) override { \
            DAS_PROFILE_NODE \
            return ImplCallStaticFunctionImm<FuncT,CTYPE>::call(*fn, context, arguments); \
    }
    DAS_EVAL_NODE
#undef EVAL_NODE
    */
};

struct ExternalFn : public ::das::ExternalFnBase
{
    __forceinline ExternalFn(skr::das::BuiltInSimProc call, const char * name, const char * cppName = nullptr)
        : ::das::ExternalFnBase(name,cppName), call(call)
    {
        
    }

    virtual ::das::SimNode* makeSimNode(::das::Context & context, const ::das::vector<::das::ExpressionPtr> & ) override 
    {
        const char * fnName = context.code->allocateName(this->name);
        return context.code->makeNode<::SimNode_ExternCall>(call, at, fnName);
    }

    virtual void* getBuiltinAddress() const override { return (void*)call; }
    skr::das::BuiltInSimProc call = nullptr;
};
}

namespace skr {
namespace das {

SimFunctionId::SimFunctionId(void* ptr) SKR_NOEXCEPT
    : ptr(ptr)
{

}

SimFunctionId::~SimFunctionId() SKR_NOEXCEPT
{
    
}

const char8_t* SimFunctionId::get_name() const
{
    auto f = (::das::SimFunction*)ptr;
    return (const char8_t*)f->name;
}

const char8_t* SimFunctionId::get_mangled_name() const
{
    auto f = (::das::SimFunction*)ptr;
    return (const char8_t*)f->mangledName;
}

const uint64_t SimFunctionId::get_mangled_name_hash() const
{
    auto f = (::das::SimFunction*)ptr;
    return f->mangledNameHash;
}

const uint64_t SimFunctionId::get_stack_size() const
{
    auto f = (::das::SimFunction*)ptr;
    return f->stackSize;
}

BuiltInFunction BuiltInFunction::_make(::das::BuiltInFunction* ptr)
{
    return BuiltInFunction(ptr);
}

::das::BuiltInFunction* BuiltInFunction::_get() const
{
    return ptr;
}

BuiltInFunction::BuiltInFunction(const BuiltInFunction& t) SKR_NOEXCEPT
{
    ptr = t.ptr;
    ptr->addRef();
}

BuiltInFunction& BuiltInFunction::operator=(const BuiltInFunction& t) SKR_NOEXCEPT
{
    ptr = t.ptr;
    ptr->addRef();
    return *this;
}

BuiltInFunction::BuiltInFunction() SKR_NOEXCEPT {}
BuiltInFunction::BuiltInFunction(std::nullptr_t) SKR_NOEXCEPT {}

BuiltInFunction::~BuiltInFunction() SKR_NOEXCEPT
{
    SKR_ASSERT(ptr->use_count());
    ptr->delRef();
}

BuiltInFunction BuiltInFunction::MakeExternFunction(const Library* lib, 
    BuiltInSimProc call, bool IS_CMRES, const char8_t* name, const char8_t* cppName) SKR_NOEXCEPT
{
    auto fnX = ::das::make_smart<::ExternalFn>(
        call, (const char*)name, (const char*)(cppName ? cppName : name));
    return BuiltInFunction::_make(fnX.orphan());
}

void BuiltInFunction::set_is_property(bool o) SKR_NOEXCEPT
{
    ptr->propertyFunction = o;
}

bool BuiltInFunction::is_property() const SKR_NOEXCEPT
{
    return ptr->propertyFunction;
}

void BuiltInFunction::set_is_call_based(bool o) SKR_NOEXCEPT
{
    ptr->callBased = o;
}

bool BuiltInFunction::is_call_based() const SKR_NOEXCEPT
{
    return ptr->callBased;
}

void BuiltInFunction::construct(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT
{
    ::das::vector<::das::TypeDeclPtr> argsV(N);
    for (uint64_t i = 0u; i < N; i++) 
    {
        argsV[i] = args[i]._get();
    }
    ptr->construct(argsV);
}

void BuiltInFunction::construct_external(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT
{
    ::das::vector<::das::TypeDeclPtr> argsV(N);
    for (uint64_t i = 0u; i < N; i++) 
    {
        argsV[i] = args[i]._get();
    }
    ptr->constructExternal(argsV);
}

void BuiltInFunction::construct_interop(const TypeDecl* args, const uint64_t N) SKR_NOEXCEPT
{
    ::das::vector<::das::TypeDeclPtr> argsV(N);
    for (uint64_t i = 0u; i < N; i++) 
    {
        argsV[i] = args[i]._get();
    }
    ptr->constructInterop(argsV);
}

BuiltInFunction& BuiltInFunction::arg(const char8_t *argName) SKR_NOEXCEPT
{
    ptr->arg((const char*)argName);
    return *this;
}

} // namespace das
} // namespace skr