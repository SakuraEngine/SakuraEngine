#include "skr_input/Interactions.h"

namespace skr::input
{

void InteractionTap_Float::Update(float value, bool IsInit)
{
    if(IsInit)
    {
        _time = 0.f;
        if(value >= _pressPoint)
            _state = InteractionState::Started;
        else
            _state = InteractionState::Waiting;
    }
    else 
    {
        switch (_state) 
        {
            case InteractionState::Waiting :
                if(value >= _pressPoint)
                    _state = InteractionState::Started;
                break;
            case InteractionState::Started : 
                if(value < _pressPoint)
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

bool InteractionTap_Float::IsTriggerByState()
{
    return _state == InteractionState::Performed;
}

}