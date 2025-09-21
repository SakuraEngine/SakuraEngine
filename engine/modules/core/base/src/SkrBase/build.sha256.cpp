#include "SkrBase/types/sha256.h"
#include "crypt/WjCryptLib_Sha256.h"

// check
static_assert(sizeof(skr::SHA256) == sizeof(SHA256_HASH), "SHA256 size mismatch");
static_assert(alignof(skr::SHA256) >= alignof(SHA256_HASH), "SHA256 align mismatch");
static_assert(sizeof(skr::SHA256Builder) >= sizeof(Sha256Context), "SHA256Builder context size mismatch");
static_assert(alignof(skr::SHA256Builder) >= alignof(Sha256Context), "SHA256Builder align mismatch");

// sha256 builder impl
namespace skr
{
// build step
void SHA256Builder::init()
{
    ::Sha256Initialise(reinterpret_cast<Sha256Context*>(payload));
}
void SHA256Builder::update(const void* str, uint64_t str_size)
{
    ::Sha256Update(
        reinterpret_cast<Sha256Context*>(payload),
        str,
        (uint32_t)str_size
    );
}
SHA256 SHA256Builder::finalize(bool reset)
{
    SHA256 result;
    ::Sha256Finalise(
        reinterpret_cast<Sha256Context*>(payload),
        reinterpret_cast<SHA256_HASH*>(&result)
    );
    if (reset) [[likely]]
    {
        init();
    }
    return result;
}
} // namespace skr