#include "SkrGui/framework/render_object/render_box.hpp"
#include "SkrGui/math/layout.hpp"

namespace skr
{
namespace gui
{
class SKR_GUI_API RenderStack : public RenderBox
{
public:
    SKR_GUI_TYPE(RenderStack, "0c1ac8b5-d3aa-4560-a011-4b655231c8ac", RenderBox);
    RenderStack(skr_gdi_device_id gdi_device);

    virtual void layout(BoxConstraint constraints, bool needSize = false) override;
    Positional   get_position(int index); // each child's corresponding positional property
    virtual void add_child(RenderObject* child) override;
    virtual void insert_child(RenderObject* child, int index) override;
    virtual void remove_child(RenderObject* child) override;
    void         set_positional(int index, Positional positional);

private:
    Array<Positional> positionals;
};
} // namespace gui
} // namespace skr