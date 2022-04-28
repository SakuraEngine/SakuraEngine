#pragma once
#include <inttypes.h>

typedef struct skr_guid_t {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[8];
} skr_guid_t;