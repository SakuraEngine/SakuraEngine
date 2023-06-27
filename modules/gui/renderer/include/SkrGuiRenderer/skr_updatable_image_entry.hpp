#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/backend/resource/surfaces.hpp"

namespace skr::gui
{
struct SkrNativeDevice;

struct SKR_GUI_RENDERER_API SkrUpdatableImageEntry : public IUpdatableImageEntry {
    SKR_GUI_OBJECT(SkrUpdatableImageEntry, "e0b0f2a0-2b0a-4b0a-9b0a-0b0a0b0a0b0a", IUpdatableImageEntry)

    NotNull<IResourceProvider*> provider() const SKR_NOEXCEPT override;
    void                        destroy_resoure(NotNull<IResource*> resource) SKR_NOEXCEPT override;
    void                        visit_requested_resources(VisitResourceFuncRef visitor) override;
    NotNull<IImage*>            request_image_of_size(Sizef show_size) SKR_NOEXCEPT override;
    void                        visit_useable_image_size(FunctionRef<void(Sizef)> visitor) const SKR_NOEXCEPT override;
    void                        update(const ImageUpdateDesc& desc) SKR_NOEXCEPT override;

private:
    SkrNativeDevice* _provider;
};

} // namespace skr::gui