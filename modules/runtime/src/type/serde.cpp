#include "SkrRT/platform/guid.hpp"
#include "SkrRT/type/type_serde.h"
#include "SkrRT/misc/fast_float.h"
#include <charconv>

void skr_type_t::FromString(void* dst, skr::string_view str, skr::type::ValueSerializePolicy* policy) const
{
    using namespace skr::type;
    if (policy)
        policy->parse(policy, str, dst, this);
    else
    {
        auto strData = (const char*)str.raw().data();
        switch (type)
        {
            case SKR_TYPE_CATEGORY_BOOL:
                if (str == u8"true")
                    *(bool*)dst = true;
                else
                    *(bool*)dst = false;
                break;
            case SKR_TYPE_CATEGORY_I32:
                std::from_chars(strData, strData + str.size(), *(int32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_I64:
                std::from_chars(strData, strData + str.size(), *(int64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U32:
                std::from_chars(strData, strData + str.size(), *(uint32_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_U64:
                std::from_chars(strData, strData + str.size(), *(uint64_t*)dst);
                break;
            case SKR_TYPE_CATEGORY_F32:
                fast_float::from_chars(strData, strData + str.size(), *(float*)dst);
                break;
            case SKR_TYPE_CATEGORY_F64:
                fast_float::from_chars(strData, strData + str.size(), *(double*)dst);
                break;
            case SKR_TYPE_CATEGORY_GUID:
                {
                    const auto guid_sv = str;
                    *(skr_guid_t*)dst = skr::guid::make_guid_unsafe(guid_sv);
                }
                break;
            case SKR_TYPE_CATEGORY_MD5:
                {
                    const auto md5_sv = str;
                    const auto success = skr_parse_md5(md5_sv.raw().data(), (skr_md5_t*)dst);
                    (void)success;
                }
                break;
            case SKR_TYPE_CATEGORY_HANDLE:
                {
                    const auto guid_sv = str;
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