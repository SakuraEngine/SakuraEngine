#include "SkrGui/framework/build_owner.hpp"

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

    void BuildOwner::schedule_build_for(Element* element) SKR_NOEXCEPT
    {
    }

    void BuildOwner::reassemble(Element* element) SKR_NOEXCEPT
    {
    }

    void BuildOwner::build_scope(Element* element) SKR_NOEXCEPT
    {
    }

    void BuildOwner::finalize_tree() SKR_NOEXCEPT
    {
    }
}
}
