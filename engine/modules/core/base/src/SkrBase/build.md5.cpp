#include "SkrBase/types/md5.hpp"
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