#include "SkrGui/window_context.hpp"

namespace skr {
namespace gui {

struct SWindowContextImpl : public SWindowContext
{
    IPlatformWindow* get_platform_window() const SKR_NOEXCEPT final
    {
        return platform_window;
    }
    
    void set_root_element(struct SRenderWindow* root) SKR_NOEXCEPT final
    {
        root_element = root;
    }   

    SRenderWindow* get_root_element() const SKR_NOEXCEPT final
    {
        return root_element;
    }

    SRenderWindow* root_element = nullptr;
    IPlatformWindow* platform_window = nullptr;
};

} }