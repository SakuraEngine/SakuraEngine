#pragma once
#include "SkrInputSystem/skr_inputsystem.configure.h"
#include "math/vector.hpp"
#include "EASTL/any.h"

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
    virtual bool IsTrigger() = 0;
    virtual eastl::any OnSendEvent() { return nullptr;};
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
    InteractionTap_Float(float duration = 0.2f, float threshold = 0.5f) : _duration(duration),_threshold(threshold) {}
    void Update(float value, bool IsInit) override;
    void Tick(float deltaTime) override;
    bool IsTrigger() override;
private:
    float _duration;
    float _threshold;
    float _time = 0.f;
};

class SKR_INPUTSYSTEM_API InteractionTap_Vector2 : public InteractionBase<math::Vector2f>
{
public:
    InteractionTap_Vector2(float duration = 0.2f, float threshold = 0.5f) : _duration(duration),_threshold(threshold) {}
    void Update(math::Vector2f value, bool IsInit) override;
    void Tick(float deltaTime) override;
    bool IsTrigger() override;
private:
    float _duration;
    float _threshold;
    float _time = 0.f;
};

enum class PressBehavior
{
    PressOnly,          //只在按下触发
    ReleaseOnly,        //只在释放触发
    PressAndRelease,    //按下释放都触发
};

enum class PressEventType
{
    Press,
    Release,
};

class SKR_INPUTSYSTEM_API InteractionPress_Float : public InteractionBase<float>
{
public:
    InteractionPress_Float(PressBehavior behavior = PressBehavior::PressOnly, float pressEventInterval = 0.f, float threshold = 0.5f) 
        : _threshold(threshold),_behavior(behavior),_pressEventInterval(pressEventInterval) {}
    void Update(float value, bool IsInit) override;
    void Tick(float deltaTime) override;
    bool IsTrigger() override;
    eastl::any OnSendEvent() override;
private:
    float _threshold;
    PressBehavior _behavior;
    float _pressEventInterval;
    float _time = 0.f;
    bool _justReleased = false;
};

class SKR_INPUTSYSTEM_API InteractionPress_Vector2 : public InteractionBase<math::Vector2f>
{
public:
    InteractionPress_Vector2(PressBehavior behavior = PressBehavior::PressOnly, float pressEventInterval = 0.f, float threshold = 0.5f) 
        : _threshold(threshold),_behavior(behavior),_pressEventInterval(pressEventInterval) {}
    void Update(math::Vector2f value, bool IsInit) override;
    void Tick(float deltaTime) override;
    bool IsTrigger() override;
    eastl::any OnSendEvent() override;
private:
    float _threshold;
    PressBehavior _behavior;
    float _pressEventInterval;
    float _time = 0.f;
    bool _justReleased = false;
};

}