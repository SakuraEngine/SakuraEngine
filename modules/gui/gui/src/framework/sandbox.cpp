#include "SkrGui/framework/sandbox.hpp"
#include "platform/memory.h"

#include "SkrGui/framework/widget_misc.hpp"
namespace skr::gui
{
void MayBeExample()
{
    // auto sandbox = SkrNew<Sandbox>();

    // sandbox->update();
    // sandbox->layout();
    // sandbox->draw();
}
}

namespace skr::gui
{
Sandbox::Sandbox(Widget* root_widget)
    : _root_widget(root_widget)
{
}

void Sandbox::update()
{
}

void Sandbox::layout()
{
}

void Sandbox::draw()
{
}

}