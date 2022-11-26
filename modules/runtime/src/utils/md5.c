#include "utils/types.h"
#include "utils/hash.h"
#if defined(__x86_64__) || defined(__i386__)
#include "platform/cpu/cpuinfo_x86.h"
#endif

const uint32_t md5_seeds[4] = { 114u, 514u, 1919u, 810u };
void skr_make_md5(const char* str, uint32_t str_size, skr_md5_t* out_md5)
{
    if (str == NULL) return;
    out_md5->a = skr_hash32(str, (uint32_t)str_size, md5_seeds[0]);
    out_md5->b = skr_hash32(str, (uint32_t)str_size, md5_seeds[1]);
    out_md5->c = skr_hash32(str, (uint32_t)str_size, md5_seeds[2]);
    out_md5->d = skr_hash32(str, (uint32_t)str_size, md5_seeds[3]);
}