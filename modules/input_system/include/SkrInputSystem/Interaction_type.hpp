#pragma once
#include "EASTL/map.h"
#include "SkrInputSystem/module.configure.h"
#include "SkrInputSystem/interactions.hpp"
#include "SkrInputSystem/controls.hpp"

namespace skr::input
{

template<typename ValueType>
class InteractionTap : public InteractionBase<ValueType>
{
public:
    InteractionTap(float duration = 0.2f, float threshold = 0.5f) : _duration(duration),_threshold(threshold) 
    {}

    void Update(ValueType value, bool IsInit, Controls& control) override
    {
        if(IsInit)
        {
            _time = 0.f;
            if(control.ThresholdReached(_threshold))
                this->_state = InteractionState::Started;
            else
                this->_state = InteractionState::Waiting;
        }
        else 
        {
            switch (this->_state) 
            {
                case InteractionState::Waiting :
                    if(control.ThresholdReached(_threshold))
                        this->_state = InteractionState::Started;
                    break;
                case InteractionState::Started : 
                    if(!control.ThresholdReached(_threshold))
                        this->_state = InteractionState::Performed;
                    break;
                case InteractionState::Performed : 
                case InteractionState::Fail : break;
            }
        }
    }

    void Tick(double deltaTime) override
    {
        switch (this->_state) 
        {
            case InteractionState::Waiting : break;
            case InteractionState::Started : 
                _time += deltaTime;
                if(_time >= _duration)
                    this->_state = InteractionState::Fail;
                break;
            case InteractionState::Performed : 
            case InteractionState::Fail : 
                this->_state = InteractionState::Waiting;
                _time = 0.f;
                break;
        }
    }

    bool IsTrigger() override
    {
        return this->_state == InteractionState::Performed;
    }

private:
    float _duration;
    float _threshold;
    double _time = 0.f;
};

enum class PressBehavior
{
    PressOnly,          //只在按下触发
    ReleaseOnly,        //只在释放触发
    PressAndRelease,    //按下释放都触发
};

enum class PressEventType
{
    None,
    Press,
    Release,
};

template<typename ValueType>
class InteractionPress : public InteractionBase<ValueType>
{
public:
    InteractionPress(PressBehavior behavior = PressBehavior::PressOnly, float pressEventInterval = 0.f, float threshold = 0.5f) 
        : _threshold(threshold),_behavior(behavior),_pressEventInterval(pressEventInterval) 
        {}

    void Update(ValueType value, bool IsInit, Controls& control) override
    {
        if(control.ThresholdReached(_threshold))
        {
            this->_state = InteractionState::Performed;
            _time = _pressEventInterval;
            _justReleased = false;
            return;
        }
        else
        {
            if(!IsInit && this->_state == InteractionState::Performed)
                _justReleased = true;
            this->_state = InteractionState::Waiting;
        }
    }

    void Tick(double deltaTime) override
    {
        if(this->_state == InteractionState::Performed && _pressEventInterval > 0.f)
            _time += deltaTime;
    }

    bool IsTrigger() override
    {
        return (this->_state == InteractionState::Performed && _behavior != PressBehavior::ReleaseOnly && _pressEventInterval <= _time) || _justReleased;
    }

    Interaction::EvendId OnSendEvent() override
    {
        auto newId = this->NewEventId();
        if(this->_state == InteractionState::Performed)
        {
            _time = 0.f;
            _eventData.emplace(newId, PressEventType::Press);
        }
        else
        {
            _justReleased = false;
            _eventData.emplace(newId, PressEventType::Release);
        }
        return newId;
    }

    void OnReleaseEvent(Interaction::EvendId eventId) override
    {
        _eventData.erase(eventId);
    }

    PressEventType GetPressEventType(Interaction::EvendId id)
    {
        auto type = _eventData.find(id);
        if(type != _eventData.end())
            return type->second;
        return PressEventType::None;
    }

private:
    float _threshold;
    PressBehavior _behavior;
    float _pressEventInterval;
    double _time = 0.f;
    bool _justReleased = false;
    eastl::map<Interaction::EvendId, PressEventType> _eventData;
};

}