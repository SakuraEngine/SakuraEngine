#pragma once
#include "EASTL/algorithm.h"
#include "SkrInputSystem/skr_inputsystem.configure.h"
#include <stdint.h>

namespace skr::input
{

class SKR_INPUTSYSTEM_API Processor
{
public:
    virtual ~Processor(){};
    // 返回是否发生变化
    virtual bool Tick(double deltaTime) { return false; };
};

template<typename ValueType>
class ProcessorBase : public Processor
{
public:
    virtual void Update(ValueType value, bool IsInit) = 0;
    ValueType GetValue() { return _value; };
protected:
    ValueType _value;
};

class SKR_INPUTSYSTEM_API ProcessorClamp : public ProcessorBase<float>
{
    ProcessorClamp(float min, float max) : _min(min),_max(max) {};

    void Update(float value, bool IsInit) override
    {
        _value = eastl::clamp(value, _min, _max);
    }

private:
    float _min;
    float _max;
};
}