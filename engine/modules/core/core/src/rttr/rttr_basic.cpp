#include <SkrRTTR/irttr_basic.hpp>

namespace skr
{
IRTTRBasic::~IRTTRBasic() = default;
const RTTRType* IRTTRBasic::StaticType()
{
    static RTTRType* type = type_of<IRTTRBasic>();
    return type;
}
} // namespace skr