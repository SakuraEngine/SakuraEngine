#include <SkrObject/object.hpp>
#include <SkrObject/object_ptr.hpp>
#include "./gc_private.hpp"

namespace skr
{
static_assert(ObjPtrBasic::kFlagMask < alignof(Object), "Flag mask may cause data loss of pointer");

// ctor & dtor
Object::Object()
{
    ObjectManager::register_object(this);
}
Object::~Object()
{
}

} // namespace skr