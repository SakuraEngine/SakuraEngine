#include "SkrAnim/resources/skin_resource.hpp"

namespace skr::resource
{
skr_guid_t SSkinFactory::GetResourceType()
{
    using namespace skr::anim;
    return skr::rttr::type_id<SkinResource>();
}
} // namespace skr::resource