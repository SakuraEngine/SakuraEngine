#include "SkrAnim/resources/skin_resource.h"

namespace skr::resource
{
skr_guid_t SSkinFactory::GetResourceType()
{
    return skr::rttr::type_id<skr_skin_resource_t>();
}
} // namespace skr::resource