#pragma once
#include "SkrGui/framework/widget/stateful_widget.hpp"
#include "SkrBase/config.h"
#include "counter_state.generated.h"

namespace skr::gui
{
struct ClickGestureRecognizer;

struct [[sattr(guid = "a0f18066-235e-41f7-bb76-34acf44511a1")]]
Fuck
{

    Array<int> int_arr;
};

struct [[sattr(guid = "d7968418-6b0b-4261-b27e-256074a6f83b"
)]] OGUI_SANDBOX_API CounterState : public State
{
    SKR_GENERATE_BODY(CounterState)

    int32_t count = 0;

    NotNull<Widget*> build(NotNull<IBuildContext*> context) SKR_NOEXCEPT override;

private:
    ClickGestureRecognizer* _click_gesture = nullptr;
};

struct [[sattr(guid = "aa98a7a6-2d8b-447c-bc6d-eb24d633cfb3"
)]] OGUI_SANDBOX_API Counter : public StatefulWidget
{
    SKR_GENERATE_BODY(Counter)
    ~Counter() = default;

    NotNull<State*> create_state() SKR_NOEXCEPT override
    {
        return SkrNew<CounterState>();
    }
};

} // namespace skr::gui
