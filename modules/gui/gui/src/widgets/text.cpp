#include "SkrGui/widgets/text.hpp"

namespace skr::gui
{
void Text::construct(Params params)
{
    _text = std::move(params.text);
}
} // namespace skr::gui