#include "types.hpp"


namespace skr {
namespace das {

FunctionId::FunctionId(void* ptr) SKR_NOEXCEPT
    : ptr(ptr)
{

}

FunctionId::~FunctionId() SKR_NOEXCEPT
{
    
}

} // namespace das
} // namespace skr