#include <SkrRTTR/script/scriptble_object.hpp>

namespace skr
{
ScriptbleObject::~ScriptbleObject()
{
    notify_mixin_core_destroyed();
    _mixin_core = nullptr;
}
} // namespace skr