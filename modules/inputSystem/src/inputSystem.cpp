#include "skr_input/inputSystem.h"
#include <vcruntime.h>

namespace skr::input 
{

InputSystem::~InputSystem()
{
    _hardwareManager.Exit();
}

void InputSystem::Init(SWindowHandle window)
{
    auto hwnd = skr_window_get_native_handle(window);
    _hardwareManager.Init(hwnd);
    _hardwareManager.SetWindowsInstance(hwnd);
    _hardwareManager.AddListener(this);
    int32_t width,height = 0;
    skr_window_get_extent(window, &width, &height);
    SetDisplaySize(width, height);

    _keyboard = _hardwareManager.CreateAndGetDevice<gainput::InputDeviceKeyboard>();
    _mouse = _hardwareManager.CreateAndGetDevice<gainput::InputDeviceMouse>();
    _builtIn = _hardwareManager.CreateAndGetDevice<gainput::InputDeviceBuiltIn>();
    _touch = _hardwareManager.CreateAndGetDevice<gainput::InputDeviceTouch>();
    UpdateAllDevice();
}

template<typename T>
void UpdateDevice(gainput::InputManager& manager, eastl::vector<T*>& allDevice)
{
    while (true) 
    {
        if(allDevice.empty())
            allDevice.push_back(manager.CreateAndGetDevice<T>());
        else if(T* device = allDevice.back();device->IsAvailable())
        {
            allDevice.push_back(manager.CreateAndGetDevice<T>(allDevice.size()));
        }
        else
            break;
    }
}

void InputSystem::UpdateAllDevice()
{
    // 检查是否有新设备
    UpdateDevice(_hardwareManager, _allPad);
}

void InputSystem::Update(double deltaTime)
{
    UpdateAllDevice();
    _hardwareManager.Update();
    for(auto& action : _allAction)
    {
        action->Tick(deltaTime);
    }
}

void InputSystem::SetDisplaySize(int32_t width, int32_t height)
{
    _hardwareManager.SetDisplaySize(width, height);
}

gainput::InputManager& InputSystem::GetHardwareManager()
{
    return _hardwareManager;
}

void InputSystem::AddInputAction(eastl::shared_ptr<InputActionBase> action)
{
    _allAction.emplace_back(action);
    action->Init(_hardwareManager);
}

bool InputSystem::OnDeviceButtonBool(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue)
{
    auto device = _hardwareManager.GetDevice(deviceId);
    DeviceInputChangeEvent event{*device, deviceButton, oldValue?1.f:0.f, newValue?1.f:0.f};
    for(auto& action : _allAction)
        action->OnDeviceInputChange(event);
    return true;
}

bool InputSystem::OnDeviceButtonFloat(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton, float oldValue, float newValue)
{
    auto device = _hardwareManager.GetDevice(deviceId);
    DeviceInputChangeEvent event{*device, deviceButton, oldValue, newValue};
    for(auto& action : _allAction)
        action->OnDeviceInputChange(event);
    return true;
}

}