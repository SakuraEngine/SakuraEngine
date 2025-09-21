#include "../common/common_device_base.hpp"
#include <SDL3/SDL_keyboard.h>
#include "SkrContainers/span.hpp"
#include <algorithm>
#include "SkrContainers/span.hpp"

namespace skr
{
extern EKeyCode TranslateSDLKeyCode(SDL_Scancode keycode);
namespace input
{

using ScanCodeBuffer = skr::InlineVector<uint8_t, 16>;
struct InputReading_SDL3Keyboard : public CommonInputReading {
    InputReading_SDL3Keyboard(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice, ScanCodeBuffer&& InScanCodes, uint64_t Timestamp) SKR_NOEXCEPT
        : CommonInputReading(pPool, pDevice),
          ScanCodes(std::move(InScanCodes)),
          Timestamp(Timestamp)
    {
    }

    bool Equal(skr::Span<uint8_t> write_span)
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
            stateArray[i].virtual_key = ScanCodes[i];
        }
        return n;
    }

    bool GetMouseState(InputMouseState* state) SKR_NOEXCEPT final
    {
        return 0;
    }

    ScanCodeBuffer ScanCodes;
    uint64_t       Timestamp;
};

struct InputDevice_SDL3Keyboard : public CommonInputDeviceBase<InputReading_SDL3Keyboard> {
    InputDevice_SDL3Keyboard(CommonInputLayer* Layer) SKR_NOEXCEPT
        : CommonInputDeviceBase<InputReading_SDL3Keyboard>(Layer)
    {
    }

    void Tick() SKR_NOEXCEPT final
    {
        skr::InlineVector<uint8_t, 16> ScanCodes;
        updateScan(ScanCodes, (uint32_t)ScanCodes.capacity());
        const auto LastReading = ReadingQueue.get();
        if (!LastReading || !LastReading->Equal({ ScanCodes.data(), ScanCodes.size() }))
        {
            if (auto old = ReadingQueue.add(
                    ReadingPool.acquire(&ReadingPool, this, std::move(ScanCodes), layer->GetCurrentTimestampUSec())
                ))
            {
                old->release();
            }
        }
    }

    const EInputKind       kinds[1] = { EInputKind::InputKindKeyboard };
    Span<const EInputKind> ReportKinds() const SKR_NOEXCEPT final
    {
        return { kinds, 1 };
    }

    bool SupportKind(EInputKind kind) const SKR_NOEXCEPT final
    {
        return kind == EInputKind::InputKindKeyboard;
    }

    static void updateScan(ScanCodeBuffer& write_span, uint32_t max_count);
};

void InputDevice_SDL3Keyboard::updateScan(ScanCodeBuffer& output, uint32_t max_count)
{
    int         numkeys;
    const bool* state    = SDL_GetKeyboardState(&numkeys);
    const auto  n        = std::min(max_count, (uint32_t)output.capacity());
    int         scancode = 0;
    for (uint32_t i = 0; scancode < numkeys && i < n; ++scancode)
    {
        if (state[scancode])
        {
            output.emplace(TranslateSDLKeyCode((SDL_Scancode)scancode));
        }
    }
}

CommonInputDevice* CreateInputDevice_SDL3Keyboard(CommonInputLayer* pLayer) SKR_NOEXCEPT
{
    InputDevice_SDL3Keyboard* pDevice = SkrNew<InputDevice_SDL3Keyboard>(pLayer);
    return pDevice;
}

} // namespace input
} // namespace skr
