#include "SkrGui/framework/render_box.hpp"


namespace skr {
namespace gui {

    class SKR_GUI_API RenderImage : public RenderBox
    {
    public:
        RenderImage(skr_gdi_device_id gdi_device);
        virtual ~RenderImage();

        virtual void layout(BoxConstraint constraints, bool needSize = false) override;
        virtual void draw(const DrawParams* params) override;

        void set_color(const skr_float4_t& color) { this->color = color; }
        skr_float4_t get_color() const { return color; }

    private:
        skr_float4_t color = { 1.0f, 1.0f, 1.0f, 1.0f };
        skr_gdi_element_id gdi_element = nullptr;
    };
}}