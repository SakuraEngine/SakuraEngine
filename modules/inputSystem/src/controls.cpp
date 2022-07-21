#include "skr_input/controls.h"
#include "EASTL/internal/copy_help.h"
#include "gainput/GainputInputDevice.h"
#include <vcruntime.h>

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

}