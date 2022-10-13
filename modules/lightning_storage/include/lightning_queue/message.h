#pragma once
#include "utils/types.h"

typedef struct SLightningMessageId {
    skr_guid_t source_instance;
    skr_guid_t message_identifier;
} SLightningMessageId;

typedef struct SLightningMessage {
    SLightningMessageId id;
    const char* queue;
    uint64_t timestamp;
    void* data;
    uint64_t data_size;
} SLightningMessage;