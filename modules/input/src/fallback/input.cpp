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

struct Input_Fallback : public Input
{
    Input_Fallback() SKR_NOEXCEPT
        : Input()
    {

    }

    ~Input_Fallback() SKR_NOEXCEPT
    {

    }
};

void Input::Initialize() SKR_NOEXCEPT
{
    instance_ = SkrNew<Input_Fallback>();
}

void Input::Finalize() SKR_NOEXCEPT
{
    SkrDelete(instance_);
}


} }