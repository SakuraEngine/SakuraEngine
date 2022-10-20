#include "skr_input/controls.h"
#include "EASTL/algorithm.h"
#include "EASTL/internal/copy_help.h"
#include "EASTL/numeric_limits.h"
#include "gainput/GainputInputDevice.h"
#include "math/scalarmath.h"
#include "utils/log.h"

namespace skr::input
{

ControlsFloat::ControlsFloat(gainput::InputDevice::DeviceType deviceType, gainput::DeviceButtonId deviceButton)
    :_type(DeviceType),_deviceType(deviceType),_deviceButton(deviceButton)
{}

ControlsFloat::ControlsFloat(gainput::InputDevice::DeviceType deviceType, unsigned index, gainput::DeviceButtonId deviceButton)
    :_type(Index),_deviceType(deviceType),_index(index),_deviceButton(deviceButton)
{}

ControlsFloat::ControlsFloat(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton)
    :_type(DeviceId),_deviceId(deviceId),_deviceButton(deviceButton)
{}

void ControlsFloat::Init(const gainput::InputManager& manager)
{
    float initValue = 0.f;
    switch (_type) 
    {
        case DeviceType:
            for (auto it = manager.begin(); it != manager.end(); ++it)
	        {
                auto device =  it->second;
		        if (device->GetType() == _deviceType)
		        {
                    float inputAxis = 0.f;
                    auto buttonType = device->GetButtonType(_deviceButton);
                    if(buttonType == gainput::BT_BOOL)
                        inputAxis = device->GetBool(_deviceButton);
                    else if(buttonType == gainput::BT_FLOAT)
                        inputAxis = device->GetFloat(_deviceButton);

                    if(inputAxis > initValue) //以最大值的设备输入为准
                        initValue = inputAxis;
		        }
	        }
            break;
        case Index:
            if(auto findDeviceId = manager.FindDeviceId(_deviceType, _index);
                findDeviceId != gainput::InvalidDeviceId)
            {
                auto device = manager.GetDevice(findDeviceId);
                if (device->GetType() == _deviceType)
		        {
			        float inputAxis = 0.f;
                    auto buttonType = device->GetButtonType(_deviceButton);
                    if(buttonType == gainput::BT_BOOL)
                        inputAxis = device->GetBool(_deviceButton);
                    else if(buttonType == gainput::BT_FLOAT)
                        inputAxis = device->GetFloat(_deviceButton);

                    initValue = inputAxis;
		        }
            }
            break;
        case DeviceId:
            if (auto device = manager.GetDevice(_deviceId))
		    {
			    float inputAxis = 0.f;
                auto buttonType = device->GetButtonType(_deviceButton);
                if(buttonType == gainput::BT_BOOL)
                    inputAxis = device->GetBool(_deviceButton);
                else if(buttonType == gainput::BT_FLOAT)
                    inputAxis = device->GetFloat(_deviceButton);
                
                initValue = inputAxis;
		    }
            break;
    }
    SetRawValue(initValue, true);
}

void ControlsFloat::OnDeviceInputChange(const DeviceInputChangeEvent& event)
{
    switch (_type) 
    {
        case DeviceType:
            if (event.device.GetType() == _deviceType && event.deviceButton == _deviceButton)
                SetRawValue(event.newValue);
            break;
        case Index:
            if (event.device.GetType() == _deviceType && event.device.GetIndex() == _index && event.deviceButton == _deviceButton)
                SetRawValue(event.newValue);
            break;
        case DeviceId:
            if (event.device.GetDeviceId() == _deviceId && event.deviceButton == _deviceButton)
                SetRawValue(event.newValue);
            break;
    }
}

void ControlsFloat::OnTick(double deltaTime)
{
    ProcessorTick(deltaTime);
    InteractionTick(deltaTime);
}

bool ControlsFloat::ThresholdReached(float threshold)
{
    return _value >= threshold;
}

void Vector2Control::Init(const gainput::InputManager& manager)
{
    for(const auto& direction : _buttonDirections)
    {
        direction.up->Init(manager);
        direction.down->Init(manager);
        direction.left->Init(manager);
        direction.right->Init(manager);
    }
    for(const auto& direction : _stickDirections)
    {
        direction.x->Init(manager);
        direction.y->Init(manager);
    }
    UpdateRawValue();
}

void Vector2Control::OnDeviceInputChange(const DeviceInputChangeEvent& event)
{
    for(const auto& direction : _buttonDirections)
    {
        direction.up->OnDeviceInputChange(event);
        direction.down->OnDeviceInputChange(event);
        direction.left->OnDeviceInputChange(event);
        direction.right->OnDeviceInputChange(event);
    }
    for(const auto& direction : _stickDirections)
    {
        direction.x->OnDeviceInputChange(event);
        direction.y->OnDeviceInputChange(event);
    }
    UpdateRawValue();
}

void Vector2Control::OnTick(double deltaTime)
{
    for(const auto& direction : _buttonDirections)
    {
        direction.up->OnTick(deltaTime);
        direction.down->OnTick(deltaTime);
        direction.left->OnTick(deltaTime);
        direction.right->OnTick(deltaTime);
    }
    for(const auto& direction : _stickDirections)
    {
        direction.x->OnTick(deltaTime);
        direction.y->OnTick(deltaTime);
    }
    UpdateRawValue();
    ProcessorTick(deltaTime);
    InteractionTick(deltaTime);
}

bool Vector2Control::ThresholdReached(float threshold)
{
    for(const auto& direction : _buttonDirections)
    {
        if(direction.up->GetValue() >= threshold)
            return true;
        if(direction.down->GetValue() >= threshold)
            return true;
        if(direction.left->GetValue() >= threshold)
            return true;
        if(direction.right->GetValue() >= threshold)
            return true;
    }
    for(const auto& direction : _stickDirections)
    {
        if(math::abs(direction.x->GetValue()) >= threshold)
            return true;
        if(math::abs(direction.y->GetValue()) >= threshold)
            return true;
    }
    return false;
}

void Vector2Control::Bind(const ButtonDirection& controls)
{
    controls.up->DisableEvent(true);
    controls.down->DisableEvent(true);
    controls.left->DisableEvent(true);
    controls.right->DisableEvent(true);
    _buttonDirections.push_back(controls);
}

void Vector2Control::Bind(const StickDirection& controls)
{
    controls.x->DisableEvent(true);
    controls.y->DisableEvent(true);
    _stickDirections.push_back(controls);
}

void Vector2Control::UpdateRawValue()
{
    float up = 0.f;
    float down = 0.f;
    float left = 0.f;
    float right = 0.f;
    for(const auto& direction : _buttonDirections)
    {
        up += direction.up->GetValue();
        down += direction.down->GetValue();
        left += direction.left->GetValue();
        right += direction.right->GetValue();
    }
    up = eastl::clamp(up, _min_y, _max_y);
    down = eastl::clamp(down, _min_y, _max_y);
    left = eastl::clamp(left, _min_x, _max_x);
    right = eastl::clamp(right, _min_x, _max_x);

    float x = 0.f;
    float y = 0.f;
    for(const auto& direction : _stickDirections)
    {
        x += direction.x->GetValue();
        y += direction.y->GetValue();
    }
    x = eastl::clamp(x, _min_x, _max_x);
    y = eastl::clamp(y, _min_y, _max_y);

    const skr_float2_t newValue{eastl::clamp(x + right - left, _min_x, _max_x), eastl::clamp(y + up - down, _min_y, _max_y)};
    // SKR_LOG_DEBUG("Vector2Control    newValue(X:%f,Y:%f) OldValue(X:%f,Y:%f)  ", newValue.X, newValue.Y, _rawValue.X, _rawValue.Y);
    SetRawValue(newValue);
}

}