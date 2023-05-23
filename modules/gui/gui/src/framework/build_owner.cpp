#include "SkrGui/framework/build_owner.hpp"
#include "misc/defer.hpp"

namespace skr
{
namespace gui
{
    BuildOwner::BuildOwner()
        : _dirty_elememts_needs_resorting(false)
    {
    }

    BuildOwner::~BuildOwner()
    {
    }

    void BuildOwner::schedule_build_for(not_null<Element*> element) SKR_NOEXCEPT
    {
        SKR_ASSERT(element->_owner == this);
        SKR_ASSERT(element->_dirty);
        if(element->_in_dirty_list)
        {
            SKR_ASSERT(_debug_is_in_build_scope);
            _dirty_elememts_needs_resorting = true;
            return;
        }
        _dirty_elements->push_back(element);
        element->_in_dirty_list = true;
    }

    void BuildOwner::reassemble(Element* element) SKR_NOEXCEPT
    {
    }

    void BuildOwner::build_scope(Element* context) SKR_NOEXCEPT
    {
        if(_dirty_elements->empty())
            return;
        SKR_ASSERT(_debug_state_lock_level >= 0);
        SKR_ASSERT(!_debug_building);
        _debug_state_lock_level++;
        _debug_building = true;
        _debug_is_in_build_scope = true;

        auto cleanup = [&]()
        {
            for(Element* element : *_dirty_elements)
            {
                SKR_ASSERT(element->_in_dirty_list);
                element->_in_dirty_list = false;
            }
            _dirty_elements->clear();
            _scheduled_flush_dirty_elements = false;
            _dirty_elememts_needs_resorting = false;
            _debug_is_in_build_scope = false;
            SKR_ASSERT(_debug_building);
            _debug_building = false;
            _debug_state_lock_level--;
            SKR_ASSERT(_debug_state_lock_level >= 0);
        };
        SKR_DEFER({ cleanup(); });

        _scheduled_flush_dirty_elements = true;
        std::sort(_dirty_elements->begin(), _dirty_elements->end(), [](Element* a, Element* b) {
            return Element::_compare_depth(a, b) < 0;
        });
        _dirty_elememts_needs_resorting = false;
        int dirty_elements_count = _dirty_elements->size();
        for(int i = 0; i < dirty_elements_count; i++)
        {
            if(_dirty_elememts_needs_resorting)
            {
                std::sort(_dirty_elements->begin(), _dirty_elements->end(), [](Element* a, Element* b) {
                    return Element::_compare_depth(a, b) < 0;
                });
                _dirty_elememts_needs_resorting = false;
                dirty_elements_count = _dirty_elements->size();
                while(i > 0 && _dirty_elements[i]->_dirty)
                {
                    // It is possible for previously dirty but inactive widgets to move right in the list.
                    // We therefore have to move the index left in the list to account for this.
                    // We don't know how many could have moved. However, we do know that the only possible
                    // change to the list is that nodes that were previously to the left of the index have
                    // now moved to be to the right of the right-most cleaned node, and we do know that
                    // all the clean nodes were to the left of the index. So we move the index left
                    // until just after the right-most clean node.
                    i--;
                }
            }
            Element* dirty_element = _dirty_elements[i];
            SKR_ASSERT(dirty_element->_in_dirty_list);
            SKR_ASSERT(dirty_element->_lifecycle_state != ElementLifecycle::active || dirty_element->_debug_is_in_scope(context));
            dirty_element->rebuild();
        }
        auto checkIfMissing = [&]() {
            for(int i = 0; i < dirty_elements_count; i++)
            {
                Element* dirty_element = _dirty_elements[i];
                if(dirty_element->_dirty && dirty_element->_lifecycle_state == ElementLifecycle::active)
                    return true;
            }
            return false;
        };
        SKR_ASSERT(!checkIfMissing());
    }

    void BuildOwner::finalize_tree() SKR_NOEXCEPT
    {
    }
}
}
