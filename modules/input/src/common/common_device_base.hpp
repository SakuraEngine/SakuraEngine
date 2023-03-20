#pragma once
#include "common_layer.hpp"
#include "reading_pool.hpp"
#include "reading_ring.hpp"

namespace skr {
namespace input {

template<typename ReadingType>
struct CommonInputDeviceBase : public CommonInputDevice
{
    CommonInputDeviceBase(struct CommonInputLayer* pLayer) SKR_NOEXCEPT
        : CommonInputDevice(pLayer)
    {

    }
    virtual ~CommonInputDeviceBase() SKR_NOEXCEPT = default;

    EInputResult GetCurrentReading(EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (!out_reading) return INPUT_RESULT_FAIL;
        if (!this->SupportKind(kind))
        {
            return INPUT_RESULT_NOT_FOUND;
        }

        const auto LastReading = ReadingQueue.get();
        if (!LastReading || LastReading == nullptr)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        LastReading->Fill(out_reading);
        return INPUT_RESULT_OK;
    }

    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (!this->SupportKind(kind)) return INPUT_RESULT_NOT_FOUND;
        if (!out_reading) return INPUT_RESULT_FAIL;

        const auto LastReading = ReadingQueue.get();
        if (!LastReading)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        if (reference == nullptr)
        {
            ReadingQueue.get()->Fill(out_reading);
            return INPUT_RESULT_OK;
        }

        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        if (uint64_t count = ReadingQueue.get_size())
        {
            for (uint64_t i = 0; i < count; ++i)
            {
                auto Reading = ReadingQueue.get(i);
                if (Reading && Reading->GetTimestamp() > timestamp)
                {
                    Reading->Fill(out_reading);
                    return INPUT_RESULT_OK;
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (!this->SupportKind(kind)) return INPUT_RESULT_NOT_FOUND;
        if (!out_reading) return INPUT_RESULT_FAIL;
    
        if (reference == nullptr)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
    
        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        if (uint64_t count = ReadingQueue.get_size())
        {
            for (uint64_t i = count - 1; i > 0; i--)
            {
                auto Reading = ReadingQueue.get(i);
                if (Reading->GetTimestamp() < timestamp)
                {
                    Reading->Fill(out_reading);
                    return INPUT_RESULT_OK;
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    ReadingRing<ReadingType*> ReadingQueue;
    CommonInputReadingPool<ReadingType> ReadingPool;
};

} }