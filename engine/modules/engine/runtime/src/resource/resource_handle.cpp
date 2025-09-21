#include "SkrBase/misc/debug.h"
#include "SkrRuntime/resource/resource_handle.h"
#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrRuntime/resource/resource_system.h"

static constexpr uint64_t kResourceHandleRecordMask        = ~(alignof(SResourceRecord) - 1);
static constexpr uint64_t kResourceHandleRequesterTypeMask = alignof(SResourceRecord) - 1;

SResourceHandle::SResourceHandle()
{
    std::memset((void*)this, 0, sizeof(SResourceHandle));
}

SResourceHandle::~SResourceHandle()
{
    // TODO: is this OK?
    reset();
}

SResourceHandle::SResourceHandle(const skr::GUID& other)
{
    guid = other;
    SKR_ASSERT(padding != 0 || is_null());
}

SResourceHandle::SResourceHandle(const SResourceHandle& other)
{
    guid = other.get_serialized();
    SKR_ASSERT(padding != 0 || is_null());
}

SResourceHandle::SResourceHandle(SResourceHandle&& other)
{
    memcpy((void*)this, &other, sizeof(SResourceHandle));
    memset((void*)&other, 0, sizeof(SResourceHandle));
}

SResourceHandle& SResourceHandle::operator=(const SResourceHandle& other)
{
    set_guid(other.get_serialized());
    return *this;
}

SResourceHandle::SResourceHandle(const SResourceHandle& other, uint64_t inRequester, ESkrRequesterType requesterType)
{
    if (other.is_null())
        std::memset((void*)this, 0, sizeof(SResourceHandle));
    if (other.padding != 0)
        guid = other.guid;
    auto record = other.get_record();
    SKR_ASSERT(record);
    requesterId = record->AddReference(inRequester, requesterType);
    pointer     = (uint64_t)record | (uint64_t(requesterType) & kResourceHandleRequesterTypeMask);
}

SResourceHandle& SResourceHandle::operator=(const skr::GUID& other)
{
    set_guid(other);
    return *this;
}

SResourceHandle& SResourceHandle::operator=(SResourceHandle&& other)
{
    memcpy((void*)this, &other, sizeof(SResourceHandle));
    memset((void*)&other, 0, sizeof(SResourceHandle));
    return *this;
}

void SResourceHandle::set_ptr(void* ptr)
{
    reset();
    auto system = skr::GetResourceSystem();
    auto record = system->_GetRecord(ptr);
    if (!record)
        return;
    set_record(record);
}

void SResourceHandle::set_guid(const skr::GUID& inGUID)
{
    reset();
    guid = inGUID;
    SKR_ASSERT(padding != 0 || is_null());
}

bool SResourceHandle::is_resolved() const
{
    return padding == 0 && get_resolved() != nullptr;
}

void* SResourceHandle::get_ptr() const
{
    SKR_ASSERT(padding == 0);
    const auto record = get_record();
    return record != nullptr ? record->resource : nullptr;
}

skr::GUID SResourceHandle::get_guid() const
{
    SKR_ASSERT(padding != 0);
    return guid;
}

skr::GUID SResourceHandle::get_type() const
{
    SKR_ASSERT(padding == 0);
    const auto record = get_record();
    return record != nullptr ? record->header.type : skr::GUID();
}

void* SResourceHandle::get_resolved(bool requireInstalled) const
{
    if (is_null())
        return nullptr;
    auto record = get_record();
    if (!record)
        return nullptr;
    bool statusSatisfied = false;
    statusSatisfied |= requireInstalled && record->loadingStatus == SKR_LOADING_STATUS_INSTALLED;
    statusSatisfied |= !requireInstalled && (record->loadingStatus >= SKR_LOADING_STATUS_LOADED && record->loadingStatus < SKR_LOADING_STATUS_UNLOADING);
    if (statusSatisfied)
        return record->resource;
    return nullptr;
}

skr::GUID SResourceHandle::get_serialized() const
{
    if (is_null())
        return guid;
    if (padding != 0)
        return get_guid();
    auto record = get_record();
    SKR_ASSERT(record);
    return record->header.guid;
}

void SResourceHandle::resolve(bool requireInstalled, uint64_t inRequester, ESkrRequesterType requesterType)
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
    {
        auto system = skr::GetResourceSystem();
        system->LoadResource(*this, requireInstalled, inRequester, requesterType);
    }
}

void SResourceHandle::unload()
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
        return;
    auto system = skr::GetResourceSystem();
    system->UnloadResource(*this);
}

bool SResourceHandle::is_null() const
{
    return padding == 0 && get_record() == nullptr;
}

void SResourceHandle::reset()
{
    if (is_resolved())
        unload();
    std::memset((void*)this, 0, sizeof(SResourceHandle));
}

ESkrLoadingStatus SResourceHandle::get_status(bool resolve) const
{
    if (is_null())
        return SKR_LOADING_STATUS_UNLOADED;
    if (padding != 0)
    {
        if (resolve)
        {
            auto system = skr::GetResourceSystem();
            return system->GetResourceStatus(get_guid());
        }
        return SKR_LOADING_STATUS_UNLOADED;
    }
    auto record = get_record();
    return record->loadingStatus;
}

SResourceRecord* SResourceHandle::get_record() const
{
    return (SResourceRecord*)(pointer & kResourceHandleRecordMask);
}

uint32_t SResourceHandle::get_requester_id() const
{
    SKR_ASSERT(is_resolved());
    return requesterId;
}

ESkrRequesterType SResourceHandle::get_requester_type() const
{
    SKR_ASSERT(is_resolved());
    return (ESkrRequesterType)(pointer & kResourceHandleRequesterTypeMask);
}

void SResourceHandle::set_record(SResourceRecord* record)
{
    pointer = (uint64_t)record | (pointer & kResourceHandleRequesterTypeMask);
}

void SResourceHandle::set_resolved(SResourceRecord* record, uint32_t inRequesterId, ESkrRequesterType requesterType)
{
    reset();
    pointer     = (uint64_t)record | (uint64_t(requesterType) & kResourceHandleRequesterTypeMask);
    requesterId = inRequesterId;
}
