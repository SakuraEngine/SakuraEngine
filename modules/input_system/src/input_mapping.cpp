#include "SkrInputSystem/input_mapping.h"


namespace skr {
namespace input {

InputMapping::~InputMapping() SKR_NOEXCEPT
{

}

InputTypeId InputMapping_Keyboard::get_type() const SKR_NOEXCEPT
{
    return kInputTypeId_Keyboard;
}

InputTypeId InputMapping_Mouse::get_type() const SKR_NOEXCEPT
{
    return kInputTypeId_Mouse;
}

} }