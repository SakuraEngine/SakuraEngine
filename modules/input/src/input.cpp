#include "SkrBase/misc/debug.h"
#include "SkrOS/shared_library.hpp"
#include "SkrMemory/memory.h"
#include <SkrContainers/vector.hpp>
#include "SkrContainers/span.hpp"
#include "SkrInput/input.h"
#include "common/reading_pool.hpp"

namespace skr
{
namespace input
{

const char* CommonInputReadingPoolBase::kInputReadingMemoryPoolName = "input::reading_pool";

struct InputImplementation : public Input {
    friend struct Input;

    ~InputImplementation() SKR_NOEXCEPT
    {
        finalize();
    }

    void initialize() SKR_NOEXCEPT;
    void finalize() SKR_NOEXCEPT;

protected:
    skr::Vector<InputLayer*> layers_;
};

#ifdef SKR_INPUT_USE_GAME_INPUT
extern InputLayer* Input_GameInput_Create() SKR_NOEXCEPT;
#else
InputLayer* Input_GameInput_Create() SKR_NOEXCEPT { return nullptr; }
#endif
extern InputLayer* Input_Common_Create() SKR_NOEXCEPT;

void InputImplementation::initialize() SKR_NOEXCEPT
{
    const bool bUseGDK = false;
    if (bUseGDK)
    {
        auto game_input = Input_GameInput_Create();
        if (game_input->Initialize())
        {
            layers_.add(game_input);
        }
        else
        {
            SkrDelete(game_input);
        }
    }
    else
    {
        auto common_input = Input_Common_Create();
        if (common_input->Initialize())
        {
            layers_.add(common_input);
        }
        else
        {
            SkrDelete(common_input);
        }
    }
}

void InputImplementation::finalize() SKR_NOEXCEPT
{
    for (auto layer : layers_)
    {
        layer->Finalize();
        SkrDelete(layer);
    }
    layers_.clear();
}

// InputLayer symbols
InputLayer::~InputLayer() SKR_NOEXCEPT
{
}

// Input symbols
Input* Input::instance_ = nullptr;
Input::Input() SKR_NOEXCEPT
{
}

Input::~Input() SKR_NOEXCEPT
{
}

skr::span<InputLayer*> Input::GetLayers() SKR_NOEXCEPT
{
    auto instance = static_cast<InputImplementation*>(Input::GetInstance());
    return { instance->layers_.data(), instance->layers_.size() };
}

Input* Input::GetInstance() SKR_NOEXCEPT
{
    // SKR_ASSERT(instance_ && "Input: instance is null, maybe not initialized!");
    return instance_;
}

void Input::Initialize() SKR_NOEXCEPT
{
    InputImplementation* instance = SkrNew<InputImplementation>();
    instance->initialize();
    Input::instance_ = instance;
}

void Input::Finalize() SKR_NOEXCEPT
{
    SkrDelete(Input::instance_);
    Input::instance_ = nullptr;
}

EInputResult Input::GetCurrentReading(EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading)
{
    auto layers = GetLayers();
    for (auto layer : layers)
    {
        if (layer->GetCurrentReading(kind, device, out_reading) == INPUT_RESULT_OK)
        {
            if (out_layer) *out_layer = layer;
            return INPUT_RESULT_OK;
        }
    }
    return INPUT_RESULT_NOT_FOUND;
}

EInputResult Input::GetNextReading(InputReading* reference, EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading)
{
    auto layers = GetLayers();
    for (auto layer : layers)
    {
        if (layer->GetNextReading(reference, kind, device, out_reading) == INPUT_RESULT_OK)
        {
            if (out_layer) *out_layer = layer;
            return INPUT_RESULT_OK;
        }
    }
    return INPUT_RESULT_NOT_FOUND;
}

EInputResult Input::GetPreviousReading(InputReading* reference, EInputKind kind, InputDevice* device, InputLayer** out_layer, InputReading** out_reading)
{
    auto layers = GetLayers();
    for (auto layer : layers)
    {
        if (layer->GetPreviousReading(reference, kind, device, out_reading) == INPUT_RESULT_OK)
        {
            if (out_layer) *out_layer = layer;
            return INPUT_RESULT_OK;
        }
    }
    return INPUT_RESULT_NOT_FOUND;
}

} // namespace input
} // namespace skr