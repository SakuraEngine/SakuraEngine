#include "SkrInput/input.h"
#include "platform/memory.h"
#include "platform/debug.h"
#include "GameInput/GameInput.h"
#include "platform/shared_library.hpp"

namespace skr {
namespace input {

Input* Input::instance_ = nullptr;
Input::Input() SKR_NOEXCEPT
{

}

Input::~Input() SKR_NOEXCEPT
{

}

Input* Input::GetInstance() SKR_NOEXCEPT
{
    SKR_ASSERT(instance_ && "Input: instance is null, maybe not initialized!");
    return instance_;
}

// GameInput implementation

struct Input_GameInput : public Input
{
    Input_GameInput() SKR_NOEXCEPT
        : Input()
    {
        auto loaded0 = sl0.load("./gameinputredist.dll");
        auto loaded = sl.load("./gameinput.dll");
        if (loaded0 && loaded)
        {
            pGameInputCreate = (ProcType*)sl.getRawAddress("GameInputCreate");
            auto result = pGameInputCreate(&game_input);
            SKR_ASSERT(result == 0 && "Input: failed to create GameInput instance!");
        }
    }

    ~Input_GameInput() SKR_NOEXCEPT
    {
        if (game_input)
        {
            game_input->Release();
            sl.unload();
            sl0.unload();
        }
    }

    using ProcType = decltype(GameInputCreate);
    ProcType* pGameInputCreate = nullptr;
    skr::SharedLibrary sl0;
    skr::SharedLibrary sl;
    IGameInput* game_input = nullptr;
};

void Input::Initialize() SKR_NOEXCEPT
{
    instance_ = SkrNew<Input_GameInput>();
}

void Input::Finalize() SKR_NOEXCEPT
{
    SkrDelete(instance_);
}


} }