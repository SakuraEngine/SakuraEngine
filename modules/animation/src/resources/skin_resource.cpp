#include "SkrAnim/resources/skin_resource.h"

namespace skr::resource
{
    skr_type_id_t SSkinFactory::GetResourceType()
    {
        return skr::type::type_id<skr_skin_resource_t>::get();
    }
}