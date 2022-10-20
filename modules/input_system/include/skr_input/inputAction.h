#pragma once
#include "SkrInputSystem/module.configure.h"
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
    using ActionEvent = eastl::function<void(const ValueType& /*触发时的值*/, 
                                            ControlsBase<ValueType>* /*触发的Control*/, 
                                            Interaction* /*触发的Interaction*/,
                                            Interaction::EvendId /*由触发的Interaction给这个事件分配的ID*/)>;

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
        typename ControlsBase<ValueType>::OutEvent event;
        for(auto& control : _allControls)
        {
            while (control->PopEvnet(event)) 
            {
                for(auto& action : _allActionEvent)
                    action(event.value, control.get(), event.interaction, event.eventId);
                if(event.interaction)
                    event.interaction->OnReleaseEvent(event.eventId);
            }
        }
    }

    void AddControl(const eastl::shared_ptr<ControlsBase<ValueType>>& control)
    {
        _allControls.emplace_back(control);
    }

    void ListenEvent(ActionEvent&& event)
    {
        _allActionEvent.emplace_back(eastl::move(event));
    }

protected:
    eastl::vector<eastl::shared_ptr<ControlsBase<ValueType>>> _allControls;
    eastl::vector<ActionEvent> _allActionEvent;
};
}