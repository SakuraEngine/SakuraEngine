#include "platform/guid.hpp"
#include "type/type_serde.h"
#include "utils/fast_float.h"
#include <charconv>

void skr_type_t::FromString(void* dst, skr::string_view str, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        policy->parse(policy, str, dst, this);
    else
    {
        switch (type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                if (str == u8"true")
                    *(bool*)dst = true;
                else
                    *(bool*)dst = false;
                break;
            case SKR_TYPE_CATEGORY_I32:
                std::from_chars(str.c_str(), str.c_str() + str.size(), *(int32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_I64:
                std::from_chars(str.c_str(), str.c_str() + str.size(), *(int64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U32:
                std::from_chars(str.c_str(), str.c_str() + str.size(), *(uint32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U64:
                std::from_chars(str.c_str(), str.c_str() + str.size(), *(uint64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_F32:
                fast_float::from_chars(str.c_str(), str.c_str() + str.size(), *(float*)dst);
                break;
            case SKR_TYPE_CATEGORY_F64:
                fast_float::from_chars(str.c_str(), str.c_str() + str.size(), *(double*)dst);
                break;
            case SKR_TYPE_CATEGORY_GUID:
                {
                    const auto guid_sv = skr::string_view(str.u8_str(), str.size());
                    *(skr_guid_t*)dst = skr::guid::make_guid_unsafe(guid_sv);
                }
                break;
            case SKR_TYPE_CATEGORY_MD5:
                {
                    const auto md5_sv = skr::string_view(str.u8_str(), str.size());
                    const auto success = skr_parse_md5(md5_sv.u8_str(), (skr_md5_t*)dst);
                    (void)success;
                }
                break;
            case SKR_TYPE_CATEGORY_HANDLE:
                {
                    const auto guid_sv = skr::string_view(str.u8_str(), str.size());
                    (*(skr_resource_handle_t*)dst).set_guid(skr::guid::make_guid_unsafe(guid_sv));
                }
                break;
            case SKR_TYPE_CATEGORY_ENUM:
                ((const EnumType*)this)->FromString(dst, str);
                break;
            default:
                break;
        }
    }
}