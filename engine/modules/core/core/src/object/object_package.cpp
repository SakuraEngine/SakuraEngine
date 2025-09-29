#include <SkrObject/object_package.hpp>
#include <SkrObject/object_ptr.hpp>

namespace skr
{
static_assert(ObjPtrBasic::kFlagMask < alignof(ObjectResolver), "Flag mask may cause data loss of pointer");

}