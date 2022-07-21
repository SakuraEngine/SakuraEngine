#include "skr_input/Interactions.h"

namespace skr::input
{

void InteractionTap_Float::Update(float value, bool IsInit)
{
    if(IsInit)
    {
        _time = 0.f;
        if(value >= _pressPoint)
            _state = Started;
        else
            _state = Waiting;
    }
    else 
    {
        switch (_state) 
        {
            case Waiting :
                if(value >= _pressPoint)
                    _state = Started;
                break;
            case Started : 
                if(value < _pressPoint)
                    _state = Performed;
                break;
            case Performed : 
            case Fail : break;
        }
    }
}

void InteractionTap_Float::Tick(float deltaTime)
{
    switch (_state) 
    {
        case Waiting : break;
        case Started : 
            _time += deltaTime;
            if(_time >= _duration)
                _state = Fail;
            break;
        case Performed : 
        case Fail : 
            _state = Waiting;
            _time = 0.f;
            break;
    }
}

bool InteractionTap_Float::IsTriggerByState()
{
    return _state == Performed;
}

}