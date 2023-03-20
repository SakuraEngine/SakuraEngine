#include "SkrGui/framework/render_box.hpp"

namespace skr
{
namespace gui
{
struct Positional
{
    std::optional<float> left;
    std::optional<float> top;
    std::optional<float> right;
    std::optional<float> bottom;
    std::optional<float> width;
    std::optional<float> height;

    skr_float2_t pivot;

};
class SKR_GUI_API RenderStack : public RenderBox
{
public:
    RenderStack(skr_gdi_device_id gdi_device);
    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    Positional get_position(int index); //each child's corresponding positional property
    virtual void add_child(RenderElement* child) override;
    virtual void insert_child(RenderElement* child, int index) override;
    virtual void remove_child(RenderElement* child) override;
    void set_positional(int index, Positional positional);

private:
    VectorStorage<Positional> positionals;
};
}
} // namespace skr