#include "skr_input/Interactions.h"
#include "utils/log.h"

namespace skr::input
{

void InteractionTap_Float::Update(float value, bool IsInit)
{
    if(IsInit)
    {
        _time = 0.f;
        if(value >= _threshold)
            _state = InteractionState::Started;
        else
            _state = InteractionState::Waiting;
    }
    else 
    {
        switch (_state) 
        {
            case InteractionState::Waiting :
                if(value >= _threshold)
                    _state = InteractionState::Started;
                break;
            case InteractionState::Started : 
                if(value < _threshold)
                    _state = InteractionState::Performed;
                break;
            case InteractionState::Performed : 
            case InteractionState::Fail : break;
        }
    }
}

void InteractionTap_Float::Tick(float deltaTime)
{
    switch (_state) 
    {
        case InteractionState::Waiting : break;
        case InteractionState::Started : 
            _time += deltaTime;
            if(_time >= _duration)
                _state = InteractionState::Fail;
            break;
        case InteractionState::Performed : 
        case InteractionState::Fail : 
            _state = InteractionState::Waiting;
            _time = 0.f;
            break;
    }
}

bool InteractionTap_Float::IsTrigger()
{
    return _state == InteractionState::Performed;
}

void InteractionTap_Vector2::Update(math::Vector2f value, bool IsInit)
{
    if(IsInit)
    {
        _time = 0.f;
        if(value.length() >= _threshold)
            _state = InteractionState::Started;
        else
            _state = InteractionState::Waiting;
    }
    else 
    {
        switch (_state) 
        {
            case InteractionState::Waiting :
                if(value.length() >= _threshold)
                    _state = InteractionState::Started;
                break;
            case InteractionState::Started : 
                if(value.length() < _threshold)
                    _state = InteractionState::Performed;
                break;
            case InteractionState::Performed : 
            case InteractionState::Fail : break;
        }
    }
}

void InteractionTap_Vector2::Tick(float deltaTime)
{
    switch (_state) 
    {
        case InteractionState::Waiting : break;
        case InteractionState::Started : 
            _time += deltaTime;
            if(_time >= _duration)
                _state = InteractionState::Fail;
            break;
        case InteractionState::Performed : 
        case InteractionState::Fail : 
            _state = InteractionState::Waiting;
            _time = 0.f;
            break;
    }
}

bool InteractionTap_Vector2::IsTrigger()
{
    return _state == InteractionState::Performed;
}

void InteractionPress_Float::Update(float value, bool IsInit)
{
    if(value >= _threshold)
    {
        _state = InteractionState::Performed;
        _time = _pressEventInterval;
        _justReleased = false;
        return;
    }
    else
    {
        if(!IsInit && _state == InteractionState::Performed)
            _justReleased = true;
        _state = InteractionState::Waiting;
    }
}

void InteractionPress_Float::Tick(float deltaTime)
{
    if(_state == InteractionState::Performed && _pressEventInterval > 0.f)
        _time += deltaTime;
}

bool InteractionPress_Float::IsTrigger()
{
    return (_state == InteractionState::Performed && _behavior != PressBehavior::ReleaseOnly && _pressEventInterval <= _time) || _justReleased;
}

eastl::any InteractionPress_Float::OnSendEvent()
{
    if(_state == InteractionState::Performed)
    {
        _time = 0.f;
        return PressEventType::Press;
    }
    else
    {
        _justReleased = false;
        return PressEventType::Release;
    }
}

void InteractionPress_Vector2::Update(math::Vector2f value, bool IsInit)
{
    if(value.length() >= _threshold)
    {
        _state = InteractionState::Performed;
        _time = _pressEventInterval;
        _justReleased = false;
        return;
    }
    else
    {
        if(!IsInit && _state == InteractionState::Performed)
            _justReleased = true;
        _state = InteractionState::Waiting;
    }
}

void InteractionPress_Vector2::Tick(float deltaTime)
{
    if(_state == InteractionState::Performed && _pressEventInterval > 0.f)
        _time += deltaTime;
}

bool InteractionPress_Vector2::IsTrigger()
{
    // SKR_LOG_DEBUG("InteractionPress_Vector2::IsTrigger _state:%i _justReleased:%s _time:%f", _state, _justReleased?"true":"false", _time);
    return (_state == InteractionState::Performed && _behavior != PressBehavior::ReleaseOnly && _pressEventInterval <= _time) || _justReleased;
}

eastl::any InteractionPress_Vector2::OnSendEvent()
{
    // SKR_LOG_DEBUG("InteractionPress_Vector2::OnSendEvent _state:%i", _state);
    if(_state == InteractionState::Performed)
    {
        _time = 0.f;
        return PressEventType::Press;
    }
    else
    {
        _justReleased = false;
        return PressEventType::Release;
    }
}

}