#pragma once
#include "EASTL/internal/move_help.h"
#include "EASTL/shared_ptr.h"
#include "EASTL/vector.h"
#include "utils/types.h"
#include "SkrInputSystem/Interactions.h"
#include "SkrInputSystem/processor.h"
#include "SkrInputSystem/module.configure.h"
#include "gainput/gainput.h"

namespace skr::input
{

using Condition = eastl::function<bool()>;

struct DeviceInputChangeEvent
{
    const gainput::InputDevice& device;
    gainput::DeviceButtonId deviceButton;
    float oldValue;
    float newValue;
};

class SKR_INPUTSYSTEM_API Controls
{
public:
    virtual ~Controls(){}

    virtual void Init(const gainput::InputManager& manager) = 0;
    virtual void OnDeviceInputChange(const DeviceInputChangeEvent& event) = 0;
    virtual void OnTick(double deltaTime) = 0;
    virtual bool ThresholdReached(float threshold) = 0;
    void DisableEvent(bool IsDisable)
    {
        _disableEvent = IsDisable;
    }

protected:
    bool _disableEvent = false;
};

template<typename ValueType>
class ControlsBase : public Controls
{
public:
    struct OutEvent
    {
        ValueType value;
        Interaction* interaction;
        Interaction::EvendId eventId;
    };

    bool PopEvnet(OutEvent& out)
    {
        if(_events.empty())
            return false;
        out = eastl::move(_events.front());
        _events.erase(_events.begin());
        return true;
    }

    ValueType GetValue()
    {
        return _value;
    }

    void AddProcessor(const eastl::shared_ptr<ProcessorBase<ValueType>>& processor)
    {
        _processors.emplace_back(processor);
    };

    void RemoveProcessor(const eastl::shared_ptr<ProcessorBase<ValueType>>& processor)
    {
        auto it = eastl::find_if(_processors.begin(), _processors.end(), [&](const eastl::shared_ptr<ProcessorBase<ValueType>>& p)
        {
            return p.get() == processor.get();
        });
        if(it != _processors.end())
            _processors.erase(it);
    };

    void AddInteraction(const eastl::shared_ptr<InteractionBase<ValueType>>& interaction)
    {
        _interactions.emplace_back(interaction);
    };

    void RemoveInteraction(const eastl::shared_ptr<InteractionBase<ValueType>>& interaction)
    {
        auto it = eastl::find_if(_interactions.begin(), _interactions.end(), [&](const eastl::shared_ptr<InteractionBase<ValueType>>& i)
        {
            return i.get() == interaction.get();
        });
        if(it != _interactions.end())
            _interactions.erase(it);
    };

    template<class F>
    void AddCondition(F&& condition)
    {
        _conditions.emplace_back(std::forward<F>(condition));
    }

protected:
    void OnRawValueChange(bool isInit)
    {
        ProcessorUpdate(isInit);
    }

    void OnValueChange(bool isInit)
    {
        InteractionUpdate(isInit);
        SendOutEvent();
    }

    void SetRawValue(ValueType value, bool isInit = false)
    {
        if(value != _rawValue)
        {
            _rawValue = value;
            OnRawValueChange(isInit);
        }
    }

    void UpdateValue(bool isInit)
    {
        if(_processors.empty())
        {
            if(_rawValue != _value)
            {
                _value = _rawValue;
                OnValueChange(isInit);
            }
        }
        else 
        {
            const auto processorValue = _processors.back()->GetValue();
            if(processorValue != _value)
            {
                _value = processorValue;
                OnValueChange(isInit);
            }
        }
    }

    void ProcessorUpdate(bool isInit)
    {
        ProcessorBase<ValueType>* prevProcessor = nullptr;
        for(auto& processor : _processors)
        {
            if(prevProcessor)
                processor->Update(prevProcessor->GetValue(), isInit);
            else
                processor->Update(_rawValue, isInit);
            prevProcessor = processor.get();
        }
        UpdateValue(isInit);
    };

    void ProcessorTick(double deltaTime)
    {
        ProcessorBase<ValueType>* prevProcessor = nullptr;
        for(auto& processor : _processors)
        {
            if(processor->Tick(deltaTime))
            {
                if(prevProcessor)
                    processor->Update(prevProcessor->GetValue(), false);
                else
                    processor->Update(_rawValue, false);
            }
            prevProcessor = processor.get();
        }
        UpdateValue(false);
    };

    void InteractionUpdate(bool isInit)
    {
        for(auto& interaction : _interactions)
            interaction->Update(_value, isInit, *this);
    }

    void InteractionTick(double deltaTime)
    {
        for(auto& interaction : _interactions)
            interaction->Tick(deltaTime);
        if(!_interactions.empty())
            SendOutEvent();
    }

    bool IsConditions()
    {
        for(auto& condition : _conditions)
            if(!condition())
                return false;
        return true;
    }

    void SendOutEvent()
    {
        if(_disableEvent || !IsConditions())
            return;
        if(_interactions.empty())
        {
            _events.push_back({_value, nullptr});
        }
        else 
        {
            for(auto& interaction : _interactions)
            {
                if(interaction->IsTrigger())
                {
                    _events.push_back({_value, interaction.get(), interaction->OnSendEvent()});
                }
                if(interaction->GetState() != InteractionState::Fail)
                    break;
            }
        }
    }

    ValueType _rawValue;
    ValueType _value;
    eastl::vector<eastl::shared_ptr<ProcessorBase<ValueType>>> _processors;
    eastl::vector<eastl::shared_ptr<InteractionBase<ValueType>>> _interactions;
    eastl::vector<Condition> _conditions;
    eastl::vector<OutEvent> _events;
};

class SKR_INPUTSYSTEM_API ControlsFloat : public ControlsBase<float>
{
public:
    ControlsFloat(gainput::InputDevice::DeviceType deviceType, gainput::DeviceButtonId deviceButton);
    ControlsFloat(gainput::InputDevice::DeviceType deviceType, unsigned index, gainput::DeviceButtonId deviceButton);
    ControlsFloat(gainput::DeviceId deviceId, gainput::DeviceButtonId deviceButton);

    void Init(const gainput::InputManager& manager) override;
    void OnDeviceInputChange(const DeviceInputChangeEvent& event) override;
    void OnTick(double deltaTime) override;
    bool ThresholdReached(float threshold) override;
private:
    enum
    {
        DeviceType,
        Index,
        DeviceId
    } _type;
    gainput::InputDevice::DeviceType _deviceType;
    unsigned _index;
    gainput::DeviceId _deviceId;
    gainput::DeviceButtonId _deviceButton;
};

class SKR_INPUTSYSTEM_API Vector2Control : public ControlsBase<skr_float2_t>
{
public:
    Vector2Control(float min_x = -1.f, float max_x = 1.f, float min_y = -1.f, float max_y = 1.f)
        : _min_x(min_x),_max_x(max_x),_min_y(min_y),_max_y(max_y)
    {}

    struct ButtonDirection
    {
        eastl::shared_ptr<ControlsFloat> up;
        eastl::shared_ptr<ControlsFloat> down;
        eastl::shared_ptr<ControlsFloat> left;
        eastl::shared_ptr<ControlsFloat> right;
    };

    struct StickDirection
    {
        eastl::shared_ptr<ControlsFloat> x;
        eastl::shared_ptr<ControlsFloat> y;
    };

    void Init(const gainput::InputManager& manager) override;
    void OnDeviceInputChange(const DeviceInputChangeEvent& event) override;
    void OnTick(double deltaTime) override;
    bool ThresholdReached(float threshold) override;
    void Bind(const ButtonDirection& controls);
    void Bind(const StickDirection& controls);
private:
    void UpdateRawValue();

    eastl::vector<ButtonDirection> _buttonDirections;
    eastl::vector<StickDirection> _stickDirections;
    const float _min_x;
    const float _max_x;
    const float _min_y;
    const float _max_y;
};

}