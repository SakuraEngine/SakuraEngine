#pragma once
#include "SkrGui/framework/element.hpp"
#include "SkrGui/framework/type_tree.hpp"
#include "fwd_containers.hpp"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Element, skr_gui_element)


namespace skr
{
namespace gui
{
struct SKR_GUI_API BuildOwner : public SInterface
{
    SKR_GUI_BASE_TYPE(BuildOwner, "50fd6eab-2463-4c84-8832-822b72d7ff6c")

    BuildOwner();
    ~BuildOwner();

    // build
    void schedule_build_for(Element* element) SKR_NOEXCEPT;
    void reassemble(Element* element) SKR_NOEXCEPT;
    void build_scope(Element* element) SKR_NOEXCEPT;
    void finalize_tree() SKR_NOEXCEPT;

    // global key
    // void register_global_key(GlobalKey* key, Element* element) SKR_NOEXCEPT;
    // void unregister_global_key(GlobalKey* key, Element* element) SKR_NOEXCEPT;
    
private:
    VectorStorage<Element*> _dirty_elements;
    bool _dirty_elememts_needs_resorting;
    // FocusManager* _focus_manager;
    // HashMapStorage<GlobalKey*, Element*> _global_key_registry;
};
}
} // namespace skr