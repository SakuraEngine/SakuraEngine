#pragma once
#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/atomic.h"

namespace skr {
namespace input {

struct CommonInputReadingProxy
{
    virtual void release(struct CommonInputReading* ptr) SKR_NOEXCEPT = 0;
};

struct SKR_INPUT_API CommonInputReading 
{
    CommonInputReading(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice) SKR_NOEXCEPT;
    virtual ~CommonInputReading() SKR_NOEXCEPT;

    void add_ref()
    {
        skr_atomicu32_add_relaxed(&ref_count, 1);
    }

    int release()
    {
        skr_atomicu32_add_relaxed(&ref_count, -1);
        const auto rc = skr_atomicu32_load_acquire(&ref_count);
        if (rc == 0)
        {
            pool->release(this);
        }
        return rc;
    }

    virtual uint32_t GetKeyState(uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT = 0;
    virtual bool GetMouseState(InputMouseState* state) SKR_NOEXCEPT = 0;
    virtual uint64_t GetTimestamp() const SKR_NOEXCEPT = 0;
    virtual EInputKind GetInputKind() const SKR_NOEXCEPT = 0;

    void Fill(InputReading** output) 
    { 
        if (output) 
        {
            *output = (InputReading*)this; 
            this->add_ref();
        }
    }

    SAtomicU32 ref_count = 0;
    CommonInputReadingProxy* pool = nullptr;
    struct CommonInputDevice* device = nullptr;
};

struct SKR_INPUT_API CommonInputDevice
{
    CommonInputDevice(struct CommonInputLayer* pLayer) SKR_NOEXCEPT;
    virtual ~CommonInputDevice() SKR_NOEXCEPT;

    virtual void Tick() SKR_NOEXCEPT = 0;

    virtual lite::LiteSpan<const EInputKind> ReportKinds() const SKR_NOEXCEPT = 0;
    virtual bool SupportKind(EInputKind kind) const SKR_NOEXCEPT = 0;

    virtual EInputResult GetCurrentReading(EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT = 0;
    virtual EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT = 0;
    virtual EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT = 0;

    CommonInputLayer* layer = nullptr;
};

struct SKR_INPUT_API CommonInputLayer : public InputLayer
{
    virtual ~CommonInputLayer() SKR_NOEXCEPT;
};

} }