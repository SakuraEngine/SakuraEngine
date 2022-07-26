#pragma once
#include "SkrInputSystem/skr_inputsystem.configure.h"

namespace skr::input
{

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
    virtual ~Interaction(){};
    virtual void Tick(float deltaTime) {};
    virtual bool IsTriggerByState() = 0;
    InteractionState GetState() { return _state; };
protected:
    InteractionState _state;
};

template<typename ValueType>
class InteractionBase : public Interaction
{
public:
    virtual void Update(ValueType value, bool IsInit) = 0;
};

class SKR_INPUTSYSTEM_API InteractionTap_Float : public InteractionBase<float>
{
public:
    InteractionTap_Float(float duration = 0.2f, float pressPoint = 0.5f) : _duration(duration),_pressPoint(pressPoint) {}
    void Update(float value, bool IsInit) override;
    void Tick(float deltaTime) override;
    bool IsTriggerByState() override;
private:
    float _duration;
    float _pressPoint;
    float _time = 0.f;
};

}