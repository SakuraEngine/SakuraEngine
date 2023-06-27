#pragma once
#include "SkrGui/fwd_config.hpp"
#include "SkrGui/math/geometry.hpp"

namespace skr::gui
{
struct IResourceEntry;

struct SKR_GUI_API IResourceProvider SKR_GUI_INTERFACE_BASE {
    SKR_GUI_INTERFACE_ROOT(IResourceProvider, "15bfbb60-fb28-4adc-b322-1d85d3afe6b0")
    virtual ~IResourceProvider() = default;

    virtual bool                     support_entry(SKR_GUI_TYPE_ID entry_type) const SKR_NOEXCEPT = 0;
    virtual NotNull<IResourceEntry*> create_entry(SKR_GUI_TYPE_ID entry_type, const void* create_data) = 0;
    virtual void                     destroy_entry(NotNull<IResourceEntry*> entry) SKR_NOEXCEPT = 0;

    // helper
    template <typename TCreateData>
    NotNull<typename TCreateData::EntryType*> create_entry(const TCreateData& create_data)
    {
        auto entry = create_entry(SKR_GUI_TYPE_ID_OF_STATIC(typename TCreateData::EntryType), &create_data);
        return make_not_null(SKR_GUI_CAST_FAST<typename TCreateData::EntryType>(entry.get()));
    }

    template <typename TEntry>
    NotNull<TEntry*> create_entry()
    {
        auto entry = create_entry(SKR_GUI_TYPE_ID_OF_STATIC(TEntry), nullptr);
        return make_not_null(SKR_GUI_CAST_FAST<TEntry>(entry.get()));
    }
};
} // namespace skr::gui