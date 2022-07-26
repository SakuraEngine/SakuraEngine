#pragma once
#include "EASTL/internal/function.h"
#include "EASTL/vector.h"
#include "SkrInputSystem/skr_inputsystem.configure.h"
#include "gainput/gainput.h"
#include "skr_input/controls.h"

namespace skr::input
{

class SKR_INPUTSYSTEM_API InputActionBase
{
public:
    virtual ~InputActionBase(){};
    virtual void Init(const gainput::InputManager& manager) = 0;
    virtual void OnDeviceInputChange(const DeviceInputChangeEvent& event) = 0;
    virtual void Tick(double deltaTime) = 0;
};

template<typename ValueType>
class InputAction : public InputActionBase
{
public:
    using ActionEvent = eastl::function<void(ValueType, ControlsBase<ValueType>*, Interaction*)>;

    void Init(const gainput::InputManager& manager) override
    {
        for(auto& control : _allControls)
            control->Init(manager);
        UpdateEvent();
    }

    void OnDeviceInputChange(const DeviceInputChangeEvent& event) override
    {
        for(auto& control : _allControls)
            control->OnDeviceInputChange(event);
        UpdateEvent();
    }

    void Tick(double deltaTime) override
    {
        for(auto& control : _allControls)
            control->OnTick(deltaTime);
        UpdateEvent();
    }

    void UpdateEvent()
    {
        Interaction* outInteraction = nullptr;
        ValueType outValue;
        for(auto& control : _allControls)
        {
            while (control->PopEvnet(outInteraction, outValue)) 
            {
                for(auto& action : _allActionEvent)
                    action(outValue, control.get(), outInteraction);
            }
        }
    }

    void AddControls(const eastl::shared_ptr<ControlsBase<ValueType>>& control)
    {
        _allControls.emplace_back(control);
    }

    void ListenEvent(ActionEvent&& event)
    {
        _allActionEvent.emplace_back(event);
    }

protected:
    eastl::vector<eastl::shared_ptr<ControlsBase<ValueType>>> _allControls;
    eastl::vector<ActionEvent> _allActionEvent;
};
}