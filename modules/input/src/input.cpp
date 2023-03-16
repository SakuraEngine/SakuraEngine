#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/debug.h"
#include "platform/shared_library.hpp"
#include <containers/vector.hpp>

namespace skr {
namespace input {

struct InputImplementation : public Input
{
    friend struct Input;

    void initialize() SKR_NOEXCEPT;
    void finalize() SKR_NOEXCEPT;

protected:
    skr::vector<InputLayer*> layers_;
};

#ifdef SKR_INPUT_USE_GAME_INPUT
extern InputLayer* Input_GameInput_Create() SKR_NOEXCEPT;
#else
InputLayer* Input_GameInput_Create() SKR_NOEXCEPT { return nullptr; }
#endif
extern InputLayer* Input_Common_Create() SKR_NOEXCEPT;

void InputImplementation::initialize() SKR_NOEXCEPT
{
    auto game_input = Input_GameInput_Create();
    if (game_input->Initialize()) 
    {
        layers_.emplace_back(game_input);
    }
    else
    {
        SkrDelete(game_input);
        
        auto common_input = Input_Common_Create();
        if (common_input->Initialize()) 
        {
            layers_.emplace_back(common_input);
        }
        else
        {
            SkrDelete(common_input);
        }
    }
}

void InputImplementation::finalize() SKR_NOEXCEPT
{

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

lite::LiteSpan<InputLayer*> Input::GetLayers() SKR_NOEXCEPT
{
    auto instance = static_cast<InputImplementation*>(Input::GetInstance());
    return { instance->layers_.data(), instance->layers_.size() };
}

Input* Input::GetInstance() SKR_NOEXCEPT
{
    SKR_ASSERT(instance_ && "Input: instance is null, maybe not initialized!");
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

} }