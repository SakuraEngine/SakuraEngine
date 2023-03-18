#include "../common/common_layer.hpp"
#include "../common/reading_pool.hpp"
#include <SDL2/SDL_keyboard.h>
#include "containers/span.hpp"
#include "containers/vector.hpp"
#include <EASTL/fixed_vector.h>
#include <algorithm>

namespace skr {
namespace input {

using ScanCodeBuffer = eastl::fixed_vector<uint8_t, 16>;
struct InputReading_SDL2Keyboard : public CommonInputReading
{
    InputReading_SDL2Keyboard(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice, ScanCodeBuffer&& InScanCodes, uint64_t Timestamp) SKR_NOEXCEPT
        : CommonInputReading(pPool, pDevice), ScanCodes(std::move(InScanCodes)), Timestamp(Timestamp)
    {

    }

    bool Equal(skr::span<uint8_t> write_span)
    {
        if (ScanCodes.size() != write_span.size())
            return false;
        for (int i = 0; i < ScanCodes.size(); ++i)
        {
            if (ScanCodes[i] != write_span[i])
                return false;
        }
        return true;
    }

    uint64_t GetTimestamp() const SKR_NOEXCEPT final
    {
        return Timestamp;
    }
    
    EInputKind GetInputKind() const SKR_NOEXCEPT
    {
        return EInputKind::InputKindKeyboard;
    }

    virtual uint32_t GetKeyState(uint32_t stateArrayCount, InputKeyState* stateArray) SKR_NOEXCEPT
    {
        const auto n = std::min(stateArrayCount, (uint32_t)ScanCodes.size());
        for (uint32_t i = 0; i < n; ++i)
        {
            stateArray[i].scan_code = ScanCodes[i];
        }
        return n;
    }

    uint32_t GetMouseState(InputMouseState* state) SKR_NOEXCEPT final
    {
        return 0;
    }

    ScanCodeBuffer ScanCodes;
    uint64_t Timestamp;
};

struct InputDevice_SDL2Keyboard : public CommonInputDevice
{
    InputDevice_SDL2Keyboard(CommonInputLayer* Layer) SKR_NOEXCEPT
        : CommonInputDevice(Layer)
    {

    }

    void Tick() SKR_NOEXCEPT final
    {
        eastl::fixed_vector<uint8_t, 16> ScanCodes;
        updateScan(ScanCodes, (uint32_t)ScanCodes.capacity());
        if (!CurrentReading || !CurrentReading->Equal(ScanCodes))
        {
            CurrentReading = ReadingPool.acquire(&ReadingPool, this, std::move(ScanCodes), layer->GetCurrentTimestampUSec());
            ReadingQueue.enqueue(CurrentReading);
            skr_atomic32_add_relaxed(&ReadingQueueCount, 1);
        }
        // Clear old readings
        const auto Now = layer->GetCurrentTimestampUSec();
        const auto LifetimeUSec = layer->GetReadingHistoryLifetimeUSec();
        ReadingPool.cleanup(Now, LifetimeUSec);
        {
            CommonInputReading* ptr = nullptr;
            const auto total = skr_atomic32_load_relaxed(&ReadingQueueCount);
            size_t cycles = 0u;
            while (ReadingQueue.try_dequeue(ptr) && cycles++ < total)
            {
                if (ptr->GetTimestamp() < Now - LifetimeUSec)
                {
                    ptr->release();
                    skr_atomic32_add_relaxed(&ReadingQueueCount, -1);
                }
                else
                {
                    ReadingQueue.enqueue(ptr);
                }
            }
        }
    } 

    const EInputKind kinds[1] = { EInputKind::InputKindKeyboard };
    lite::LiteSpan<const EInputKind> ReportKinds() const SKR_NOEXCEPT final
    {
        return { kinds, 1 };
    }

    bool SupportKind(EInputKind kind) const SKR_NOEXCEPT final
    {
        return kind == EInputKind::InputKindKeyboard;
    }
    
    EInputResult GetCurrentReading(EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (!out_reading) return INPUT_RESULT_FAIL;
        if (kind != EInputKind::InputKindKeyboard)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        if (CurrentReading == nullptr)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        CurrentReading->Fill(out_reading);
        return INPUT_RESULT_OK;
    }

    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (kind != InputKindKeyboard) return INPUT_RESULT_NOT_FOUND;
        if (!out_reading) return INPUT_RESULT_FAIL;

        if (kind != EInputKind::InputKindKeyboard)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        if (reference == nullptr)
        {
            CurrentReading->Fill(out_reading);
            return INPUT_RESULT_OK;
        }

        CommonInputReading* ptr = nullptr;
        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        const auto total = ReadingQueue.size_approx();
        size_t cycles = 0u;
        while (ReadingQueue.try_dequeue(ptr) && cycles++ < total)
        {
            if (ptr->GetTimestamp() > timestamp)
            {
                skr_atomic32_add_relaxed(&ReadingQueueCount, -1);
                ptr->Fill(out_reading);
                return INPUT_RESULT_OK;
            }
            ReadingQueue.enqueue(ptr);
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    EInputResult GetPreviousReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (kind != InputKindKeyboard) return INPUT_RESULT_NOT_FOUND;
        if (!out_reading) return INPUT_RESULT_FAIL;
    
        if (kind != EInputKind::InputKindKeyboard)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        if (reference == nullptr)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
    
        CommonInputReading* ptr = nullptr;
        const auto total = skr_atomic32_load_relaxed(&ReadingQueueCount);
        size_t cycles = 0u;
        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        while (ReadingQueue.try_dequeue(ptr) && cycles++ < total)
        {
            if (ptr->GetTimestamp() < timestamp)
            {
                skr_atomic32_add_relaxed(&ReadingQueueCount, -1);
                ptr->Fill(out_reading);
                return INPUT_RESULT_OK;
            }
            ReadingQueue.enqueue(ptr);
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    static void updateScan(ScanCodeBuffer& write_span, uint32_t max_count);

    moodycamel::ConcurrentQueue<CommonInputReading*> ReadingQueue;
    SAtomic32 ReadingQueueCount = 0;
    InputReading_SDL2Keyboard* CurrentReading = nullptr;
    CommonInputReadingPool<InputReading_SDL2Keyboard> ReadingPool;
};

void InputDevice_SDL2Keyboard::updateScan(ScanCodeBuffer& output, uint32_t max_count)
{
    int numkeys;
    const uint8_t* state = SDL_GetKeyboardState(&numkeys);
    const auto n = std::min(max_count, (uint32_t)output.capacity());
    int scancode = 0;
    for (uint32_t i = 0; scancode < numkeys && i < n; ++scancode)
    {
        if (state[scancode])
        {
            output.emplace_back(scancode);
        }
    }
}

CommonInputDevice* CreateInputDevice_SDL2Keyboard(CommonInputLayer* pLayer) SKR_NOEXCEPT
{
    InputDevice_SDL2Keyboard* pDevice = SkrNew<InputDevice_SDL2Keyboard>(pLayer);
    return pDevice;
}


}}
