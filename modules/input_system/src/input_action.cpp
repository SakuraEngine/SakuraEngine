#define CONTAINER_LITE_IMPL
#include "./input_action_impl.hpp"
#include "SkrInput/input.h"

namespace skr {
namespace input {

InputValueStorage::InputValueStorage(float f) SKR_NOEXCEPT
    : type(EValueType::kFloat), v({f, 0.0f, 0.0f, 0.0f})
{

}

InputValueStorage::InputValueStorage(skr_float2_t f2) SKR_NOEXCEPT
    : type(EValueType::kFloat2), v({f2.x, f2.y, 0.0f, 0.0f})
{

}

InputValueStorage::InputValueStorage(skr_float3_t f3) SKR_NOEXCEPT
    : type(EValueType::kFloat3), v({f3.x, f3.y, f3.z, 0.0f})
{

}

InputValueStorage::InputValueStorage(bool b) SKR_NOEXCEPT
    : type(EValueType::kBool), v({b ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f})
{

}

InputValueStorage::InputValueStorage(EValueType type, skr_float4_t raw) SKR_NOEXCEPT
    : type(type), v(raw)
{

}

InputValueStorage::InputValueStorage(const InputValueStorage& rhs) SKR_NOEXCEPT
    : type(rhs.type), v(rhs.v)
{

}

EValueType InputValueStorage::get_type() const SKR_NOEXCEPT
{
    return type;
}


void InputValueStorage::reset() SKR_NOEXCEPT
{
    v = {0.0f, 0.0f, 0.0f, 0.0f};
}

bool InputValueStorage::get_float(float& out_f) const SKR_NOEXCEPT
{
    if (type == EValueType::kFloat)
    {
        out_f = v.x;
        return true;
    }
    return false;
}

bool InputValueStorage::get_float2(skr_float2_t& out_f2) const SKR_NOEXCEPT
{
    if (type == EValueType::kFloat2)
    {
        out_f2.x = v.x;
        out_f2.y = v.y;
        return true;
    }
    return false;
}

bool InputValueStorage::get_float3(skr_float3_t& out_f3) const SKR_NOEXCEPT
{
    if (type == EValueType::kFloat3)
    {
        out_f3.x = v.x;
        out_f3.y = v.y;
        out_f3.z = v.z;
        return true;
    }
    return false;
}

bool InputValueStorage::get_bool(bool& out_b) const SKR_NOEXCEPT
{
    if (type == EValueType::kBool)
    {
        out_b = v.x != 0.0f;
        return true;
    }
    return false;
}

float InputValueStorage::get_magnitude_val() const SKR_NOEXCEPT
{
    switch (type)
    {
    case EValueType::kFloat:
        return v.x * v.x;
    case EValueType::kFloat2:
        return v.x * v.x + v.y * v.y;
    case EValueType::kFloat3:
        return v.x * v.x + v.y * v.y + v.z * v.z;
    case EValueType::kBool:
        return v.x * v.x;
    default:
        return 0.0f;
    }
}

InputValueStorage::~InputValueStorage() SKR_NOEXCEPT
{
    /*
    if (lowlevel.layer && lowlevel.reading)
    {
        lowlevel.layer->Release(lowlevel.reading);
    }
    */
}

InputAction::~InputAction() SKR_NOEXCEPT
{

}

InputActionImpl::~InputActionImpl() SKR_NOEXCEPT
{

}

template<>
ActionEventId InputAction::bind_event<float>(const ActionEvent<float>& event, ActionEventId id) SKR_NOEXCEPT
{
    auto _this = (InputActionImpl*)this;
    ActionEventImpl<float> func(event);
    return _this->bind_event_impl([func](const InputValueStorage& ev){
        float v;
        if (ev.get_type() == EValueType::kFloat && ev.get_float(v))
        {
            func(v);
        }
    }, id);
}

template<>
ActionEventId InputAction::bind_event<skr_float2_t>(const ActionEvent<skr_float2_t>& event, ActionEventId id) SKR_NOEXCEPT
{
    auto _this = (InputActionImpl*)this;
    ActionEventImpl<skr_float2_t> func(event);
    return _this->bind_event_impl([func](const InputValueStorage& ev){
        skr_float2_t v;
        if (ev.get_type() == EValueType::kFloat2 && ev.get_float2(v))
        {
            func(v);
        }
    }, id);
}

template<>
ActionEventId InputAction::bind_event<skr_float3_t>(const ActionEvent<skr_float3_t>& event, ActionEventId id) SKR_NOEXCEPT
{
    auto _this = (InputActionImpl*)this;
    ActionEventImpl<skr_float3_t> func(event);
    return _this->bind_event_impl([func](const InputValueStorage& ev){
        skr_float3_t v;
        if (ev.get_type() == EValueType::kFloat3 && ev.get_float3(v))
        {
            func(v);
        }
    }, id);
}

template<>
ActionEventId InputAction::bind_event<bool>(const ActionEvent<bool>& event, ActionEventId id) SKR_NOEXCEPT
{
    auto _this = (InputActionImpl*)this;
    ActionEventImpl<bool> func(event);
    return _this->bind_event_impl([func](const InputValueStorage& ev){
        bool v;
        if (ev.get_type() == EValueType::kBool && ev.get_bool(v))
        {
            func(v);
        }
    }, id);
}

} }