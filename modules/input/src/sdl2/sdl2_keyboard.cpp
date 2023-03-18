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
        if (!ReadingQueue.size() || !ReadingQueue.back()->Equal(ScanCodes))
        {
            ReadingQueue.emplace_back(
                ReadingPool.acquire(&ReadingPool, this, std::move(ScanCodes), layer->GetCurrentTimestampUSec())
            );
        }
        // Clear old readings
        const auto Now = layer->GetCurrentTimestampUSec();
        const auto LifetimeUSec = layer->GetReadingHistoryLifetimeUSec();
        ReadingPool.cleanup(Now, LifetimeUSec);
        {
            for (auto& ptr : ReadingQueue)
            {
                if (ptr->GetTimestamp() < Now - LifetimeUSec)
                {
                    ptr->release();
                    ptr = nullptr;
                }
            }
            ReadingQueue.erase(eastl::remove(ReadingQueue.begin(), ReadingQueue.end(), nullptr), ReadingQueue.end());
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
        if (!ReadingQueue.size() || ReadingQueue.back() == nullptr)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        ReadingQueue.back()->Fill(out_reading);
        return INPUT_RESULT_OK;
    }

    EInputResult GetNextReading(InputReading* reference, EInputKind kind, InputReading** out_reading) SKR_NOEXCEPT final
    {
        if (kind != InputKindKeyboard) return INPUT_RESULT_NOT_FOUND;
        if (!out_reading) return INPUT_RESULT_FAIL;

        if (!ReadingQueue.size() || kind != EInputKind::InputKindKeyboard)
        {
            return INPUT_RESULT_NOT_FOUND;
        }
        if (reference == nullptr)
        {
            ReadingQueue.back()->Fill(out_reading);
            return INPUT_RESULT_OK;
        }

        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        for (size_t i = 0; i < ReadingQueue.size(); ++i)
        {
            if (ReadingQueue[i]->GetTimestamp() > timestamp)
            {
                ReadingQueue[i]->Fill(out_reading);
                return INPUT_RESULT_OK;
            }
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
    
        auto timestamp = ((CommonInputReading*)reference)->GetTimestamp();
        if (ReadingQueue.size())
        {
            for (size_t i = ReadingQueue.size() - 1; i > 0; i--)
            {
                if (ReadingQueue[i]->GetTimestamp() < timestamp)
                {
                    ReadingQueue[i]->Fill(out_reading);
                    return INPUT_RESULT_OK;
                }
            }
        }
        return INPUT_RESULT_NOT_FOUND;
    }

    static void updateScan(ScanCodeBuffer& write_span, uint32_t max_count);

    skr::vector<InputReading_SDL2Keyboard*> ReadingQueue;
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
