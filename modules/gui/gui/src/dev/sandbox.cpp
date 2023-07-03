#include "SkrGui/dev/sandbox.hpp"
#include "platform/memory.h"
#include "SkrGui/framework/pipeline_owner.hpp"
#include "SkrGui/framework/build_owner.hpp"

namespace skr::gui
{
Sandbox::Sandbox(INativeDevice* device, ICanvasService* canvas_service, ITextService* text_service) SKR_NOEXCEPT
    : _device(device),
      _canvas_service(canvas_service),
      _text_service(text_service)
{
}

void Sandbox::init()
{
    // init owner
    _build_owner = SkrNew<BuildOwner>();
    _pipeline_owner = SkrNew<PipelineOwner>();
}
void Sandbox::shutdown()
{
}

void Sandbox::set_content(NotNull<Widget*> content)
{
    _content = content;
}
void Sandbox::show(const WindowDesc& desc)
{
    // init root render native window
    // 这里之所以要线初始化 render, 是为了做 PipelineOwner 的先行绑定

    // init widget/element bind
}

} // namespace skr::gui