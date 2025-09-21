#include "SkrAnim/resources/skin_resource.hpp"

namespace skr
{
GUID SkinFactory::GetResourceType()
{
    using namespace skr;
    return skr::type_id_of<SkinResource>();
}
} // namespace skr