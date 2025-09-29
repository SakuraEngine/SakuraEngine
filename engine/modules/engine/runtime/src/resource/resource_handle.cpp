#include "SkrBase/misc/debug.h"
#include "SkrRuntime/resource/resource_handle.h"
#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrRuntime/resource/resource_system.hpp"

static constexpr uint64_t kResourceHandleRecordMask = ~(alignof(SResourceRecord) - 1);
static constexpr uint64_t kResourceHandleRequesterTypeMask = alignof(SResourceRecord) - 1;

SResourceHandle::SResourceHandle()
{
    std::memset((void*)this, 0, sizeof(SResourceHandle));
}

SResourceHandle& SResourceHandle::operator=(const SResourceHandle& other)
{
    if (other.is_null())
    {
        std::memset((void*)this, 0, sizeof(SResourceHandle));
    }
    else if (other.is_guid())
    {
        guid = other.guid;
    }
    else if (auto record = other.get_record())
    {
        acquire_record(record);
    }
    return *this;
}

SResourceHandle::SResourceHandle(const SResourceHandle& other)
    : padding(0)
    , pointer(0)
{
    if (other.is_null())
    {
        std::memset((void*)this, 0, sizeof(SResourceHandle));
    }
    else if (other.is_guid())
    {
        guid = other.guid;
    }
    else if (auto record = other.get_record())
    {
        acquire_record(record);
    }
}

SResourceHandle::SResourceHandle(const skr::GUID& other)
    : padding(0)
    , pointer(0)
{
    guid = other;
    SKR_ASSERT(is_guid() || is_null());
}

SResourceHandle& SResourceHandle::operator=(const skr::GUID& other)
{
    if (padding == 0 && pointer != 0)
    {
        if (auto record = get_record())
        {
            record->RemoveReference();
        }
    }
    guid = other;
    SKR_ASSERT(is_guid() || is_null());
    return *this;
}

SResourceHandle::SResourceHandle(SResourceHandle&& other)
    : padding(0)
    , pointer(0)
{
    memcpy((void*)this, &other, sizeof(SResourceHandle));
    memset((void*)&other, 0, sizeof(SResourceHandle));
}

SResourceHandle& SResourceHandle::operator=(SResourceHandle&& other)
{
    memcpy((void*)this, &other, sizeof(SResourceHandle));
    memset((void*)&other, 0, sizeof(SResourceHandle));
    return *this;
}

SResourceHandle::~SResourceHandle()
{
    reset();
}

SResourceHandle::operator bool() const
{
    return !is_null();
}

void SResourceHandle::reset()
{
    if (padding == 0 && pointer != 0)
    {
        if (auto record = get_record())
        {
            record->RemoveReference();
        }
    }
    std::memset((void*)this, 0, sizeof(SResourceHandle));
}

void* SResourceHandle::load()
{
    return load(false);
}

void* SResourceHandle::install()
{
    return load(true);
}

void* SResourceHandle::get_loaded() const
{
    if (auto record = get_record())
    {
        if (record->loadingStatus >= EResourceLoadingStatus::Loaded)
            return record->resource;
    }
    return nullptr;
}

void* SResourceHandle::get_installed() const
{
    if (auto record = get_record())
    {
        if (record->loadingStatus >= EResourceLoadingStatus::Installed)
            return record->resource;
    }
    return nullptr;
}

bool SResourceHandle::is_null() const
{
    return padding == 0 && pointer == 0;
}

bool SResourceHandle::is_guid() const
{
    return padding != 0;
}

bool SResourceHandle::is_loaded() const
{
    return get_status() == EResourceLoadingStatus::Loaded;
}

bool SResourceHandle::is_installed() const
{
    return get_status() == EResourceLoadingStatus::Installed;
}

skr::GUID SResourceHandle::get_guid() const
{
    if (is_guid())
    {
        return guid;
    }
    else if (const auto record = get_record())
    {
        return record->header.guid;
    }
    return {};
}

skr::GUID SResourceHandle::get_type() const
{
    SKR_ASSERT(padding == 0);
    if (const auto record = get_record())
    {
        return record != nullptr ? record->header.type : skr::GUID();
    }
    return {};
}

EResourceLoadingStatus SResourceHandle::get_status() const
{
    if (is_null())
    {
        return EResourceLoadingStatus::Unloaded;
    }
    else if (auto record = get_record())
    {
        return record->loadingStatus;
    }
    return EResourceLoadingStatus::Unloaded;
}

SResourceHandle SResourceHandle::clone()
{
    return *this;
}

SResourceHandle SResourceHandle::clone(struct sugoi_storage_t* requester)
{
    return *this;
}

void* SResourceHandle::load(bool requireInstalled)
{
    auto system = skr::GetResourceSystem();
    if (is_null())
    {
        return nullptr;
    }
    else if (is_guid()) // new acquire
    {
        auto record = system->FindResourceRecord(guid);
        if (record == nullptr)
        {
            record = system->LoadResource(*this, requireInstalled);
        }
        if (record != nullptr)
        {
            acquire_record(record);
            if (record->loadingStatus >= EResourceLoadingStatus::Loaded)
            {
                return record->resource;
            }
        }
    }
    else if (auto record = get_record())
    {
        if (record->loadingStatus >= EResourceLoadingStatus::Loaded)
            return record->resource;
    }
    return nullptr;
}

SResourceRecord* SResourceHandle::get_record() const
{
    if (is_null())
    {
        return nullptr;
    }
    else if (is_guid())
    {
        auto system = skr::GetResourceSystem();
        return system->FindResourceRecord(guid);
    }
    return (SResourceRecord*)(pointer & kResourceHandleRecordMask);
}

void SResourceHandle::acquire_record(SResourceRecord* record) const
{
    if (pointer != (uint64_t)record)
    {
        if (padding == 0 && pointer != 0)
        {
            get_record()->RemoveReference();
            pointer = 0;
        }
        record->AddReference();
        padding = 0;
        pointer = (uint64_t)record;
    }
}
