#pragma once
#include "SkrInputSystem/module.configure.h"
#include "gainput/gainput.h"
#include "platform/window.h"
#include <EASTL/vector.h>
#include "SkrInputSystem/input_action.hpp"

namespace skr 
{
namespace input
{

class SKR_INPUTSYSTEM_API InputSystem : public gainput::InputListener
{
public:
    InputSystem() = default;
    ~InputSystem();

    void Init(SWindowHandle window);
    void Update(double deltaTime /*秒*/);
    void SetDisplaySize(int32_t width, int32_t height);
    gainput::InputManager& GetHardwareManager();

    void AddInputAction(const eastl::shared_ptr<InputActionBase>& action);

    bool OnDeviceButtonBool(gainput::DeviceId device, gainput::DeviceButtonId deviceButton, bool oldValue, bool newValue) override;
    bool OnDeviceButtonFloat(gainput::DeviceId device, gainput::DeviceButtonId deviceButton, float oldValue, float newValue) override;

private:
    InputSystem(const InputSystem &);
	InputSystem& operator=(const InputSystem &);
    void UpdateAllDevice();

    gainput::InputManager _hardwareManager;
    gainput::InputDeviceKeyboard* _keyboard;
    gainput::InputDeviceMouse* _mouse;
    eastl::vector<gainput::InputDevicePad*> _allPad;
    gainput::InputDeviceBuiltIn* _builtIn;
    gainput::InputDeviceTouch* _touch;

    eastl::vector<eastl::shared_ptr<InputActionBase>> _allAction;
    bool _initialized = false;
};

}
}