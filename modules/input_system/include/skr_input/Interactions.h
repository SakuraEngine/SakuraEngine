#pragma once
#include "SkrInputSystem/skr_inputsystem.configure.h"
#include "math/vector.hpp"
#include <stdint.h>

namespace skr::input
{

class Controls;

enum class InteractionState
{
    Waiting,    //等待预期输入
    Started,    //开始交互
    Performed,  //达成交互条件，可能是持续性的，比如按住
    Fail,       //交互失败或者超时
};

class SKR_INPUTSYSTEM_API Interaction
{
public:
    using EvendId = uint64_t;

    virtual ~Interaction(){};
    virtual void Tick(float deltaTime) {};
    virtual bool IsTrigger() = 0;
    virtual EvendId OnSendEvent() { return NewEventId(); };
    InteractionState GetState() { return _state; };
    EvendId NewEventId()
    {
        return _nextEventId++;
    }
protected:
    InteractionState _state = InteractionState::Waiting;
    EvendId _nextEventId = 0;
};

template<typename ValueType>
class InteractionBase : public Interaction
{
public:
    virtual void Update(ValueType value, bool IsInit, Controls& control) = 0;
};

}