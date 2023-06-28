#pragma once
#include "SkrGui/backend/text/text_service.hpp"

namespace skr::gui
{
struct IEmbeddedTextServiceResourceProvider;

struct _EmbeddedTextService : public ITextService {
    SKR_GUI_INTERFACE(_EmbeddedTextService, "758e0890-7030-486f-8593-5f3d6abc8c67", ITextService)

    _EmbeddedTextService(IEmbeddedTextServiceResourceProvider* provider);
    ~_EmbeddedTextService();

    NotNull<IParagraph*> create_paragraph() override;
    void                 destroy_paragraph(NotNull<IParagraph*> paragraph) override;
};

} // namespace skr::gui