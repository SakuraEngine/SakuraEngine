#pragma once
#include "SkrGui/framework/element.hpp"
#include "SkrGui/framework/type_tree.hpp"
#include "fwd_containers.hpp"
#include "platform/configure.h"

SKR_DECLARE_TYPE_ID_FWD(skr::gui, Element, skr_gui_element)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, GlobalKey, skr_gui_global_key)
SKR_DECLARE_TYPE_ID_FWD(skr::gui, FocusManager, skr_gui_focus_manager)

namespace skr
{
namespace gui
{
struct SKR_GUI_API BuildOwner
{
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
    
    VectorStorage<Element*> _dirty_elements;
    bool _dirty_elememts_needs_resorting;
    VectorStorage<Element*> _inactive_elements;
    FocusManager* _focus_manager;
    // HashMapStorage<GlobalKey*, Element*> _global_key_registry;
};
}
} // namespace skr