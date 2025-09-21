#include "SkrBase/types/md5.h"
#include "crypt/WjCryptLib_Md5.h"

// check
static_assert(sizeof(skr::MD5) == sizeof(MD5_HASH), "MD5 size mismatch");
static_assert(alignof(skr::MD5) >= alignof(MD5_HASH), "MD5 align mismatch");
static_assert(sizeof(skr::MD5Builder) >= sizeof(Md5Context), "MD5Builder context size mismatch");
static_assert(alignof(skr::MD5Builder) >= alignof(Md5Context), "MD5Builder align mismatch");

// md5 builder impl
namespace skr
{
// build step
void MD5Builder::init()
{
    ::Md5Initialise(reinterpret_cast<Md5Context*>(payload));
}
void MD5Builder::update(const void* data, uint64_t data_len)
{
    ::Md5Update(
        reinterpret_cast<Md5Context*>(payload),
        data,
        (uint32_t)data_len
    );
}
MD5 MD5Builder::finalize(bool reset)
{
    MD5 result;
    ::Md5Finalise(
        reinterpret_cast<Md5Context*>(payload),
        reinterpret_cast<MD5_HASH*>(&result)
    );
    if (reset) [[likely]]
    {
        init();
    }
    return result;
}
} // namespace skr

// capi
SKR_EXTERN_C bool skr_parse_md5(const char8_t* str32, skr_md5_t* out_md5)
{
    auto opt_md5 = skr::MD5::DecodeAuto(str32);
    if (opt_md5) [[likely]]
    {
        *out_md5 = *opt_md5;
        return true;
    }
    else
    {
        return false;
    }
}
SKR_EXTERN_C void skr_make_md5(const char8_t* str, uint32_t str_size, skr_md5_t* out_md5)
{
    *out_md5 = skr::MD5::Build(str, str_size);
}
