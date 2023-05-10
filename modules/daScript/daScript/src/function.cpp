#include "types.hpp"


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

/*
Function::~Function() SKR_NOEXCEPT
{
    
}
*/
} // namespace das
} // namespace skr