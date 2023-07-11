#include "SkrRT/resource/resource_handle.h"
#include "SkrRT/platform/debug.h"
#include "SkrRT/misc/types.h"
#include "SkrRT/resource/resource_header.hpp"
#include "SkrRT/resource/resource_system.h"

static constexpr uint64_t kResourceHandleRecordMask = ~(alignof(skr_resource_record_t) - 1);
static constexpr uint64_t kResourceHandleRequesterTypeMask = alignof(skr_resource_record_t) - 1;

skr_resource_handle_t::skr_resource_handle_t()
{
    std::memset(this, 0, sizeof(skr_resource_handle_t));
}

skr_resource_handle_t::~skr_resource_handle_t()
{
    // TODO: is this OK?
    reset();
}

skr_resource_handle_t::skr_resource_handle_t(const skr_guid_t& other)
{
    guid = other;
    SKR_ASSERT(padding != 0 || is_null());
}

skr_resource_handle_t::skr_resource_handle_t(const skr_resource_handle_t& other)
{
    guid = other.get_serialized();
    SKR_ASSERT(padding != 0 || is_null());
}

skr_resource_handle_t::skr_resource_handle_t(skr_resource_handle_t&& other)
{
    memcpy(this, &other, sizeof(skr_resource_handle_t));
    memset(&other, 0, sizeof(skr_resource_handle_t));
}

skr_resource_handle_t& skr_resource_handle_t::operator=(const skr_resource_handle_t& other)
{
    set_guid(other.get_serialized());
    return *this;
}

skr_resource_handle_t::skr_resource_handle_t(const skr_resource_handle_t& other, uint64_t inRequester, ESkrRequesterType requesterType)
{
    if (other.is_null())
        std::memset(this, 0, sizeof(skr_resource_handle_t));
    if (other.padding != 0)
        guid = other.guid;
    auto record = other.get_record();
    SKR_ASSERT(record);
    requesterId = record->AddReference(inRequester, requesterType);
    pointer = (uint64_t)record | (uint64_t(requesterType) & kResourceHandleRequesterTypeMask);
}

skr_resource_handle_t& skr_resource_handle_t::operator=(const skr_guid_t& other)
{
    set_guid(other);
    return *this;
}

skr_resource_handle_t& skr_resource_handle_t::operator=(skr_resource_handle_t&& other)
{
    memcpy(this, &other, sizeof(skr_resource_handle_t));
    memset(&other, 0, sizeof(skr_resource_handle_t));
    return *this;
}

void skr_resource_handle_t::set_ptr(void* ptr)
{
    reset();
    auto system = skr::resource::GetResourceSystem();
    auto record = system->_GetRecord(ptr);
    if (!record)
        return;
    set_record(record);
}

void skr_resource_handle_t::set_guid(const skr_guid_t& inGUID)
{
    reset();
    guid = inGUID;
    SKR_ASSERT(padding != 0 || is_null());
}

bool skr_resource_handle_t::is_resolved() const
{
    return padding == 0 && get_record() != nullptr;
}

void* skr_resource_handle_t::get_ptr() const
{
    SKR_ASSERT(padding == 0);
    const auto record = get_record();
    return record != nullptr ? record->resource : nullptr;
}

skr_guid_t skr_resource_handle_t::get_guid() const
{
    SKR_ASSERT(padding != 0);
    return guid;
}

skr_guid_t skr_resource_handle_t::get_type() const
{
    SKR_ASSERT(padding == 0);
    const auto record = get_record();
    return record != nullptr ? record->header.type : skr_guid_t();
}

void* skr_resource_handle_t::get_resolved(bool requireInstalled) const
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

skr_guid_t skr_resource_handle_t::get_serialized() const
{
    if (is_null())
        return guid;
    if (padding != 0)
        return get_guid();
    auto record = get_record();
    SKR_ASSERT(record);
    return record->header.guid;
}

void skr_resource_handle_t::resolve(bool requireInstalled, uint64_t inRequester, ESkrRequesterType requesterType)
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
    {
        auto system = skr::resource::GetResourceSystem();
        system->LoadResource(*this, requireInstalled, inRequester, requesterType);
    }
}

void skr_resource_handle_t::unload()
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
        return;
    auto system = skr::resource::GetResourceSystem();
    system->UnloadResource(*this);
}

bool skr_resource_handle_t::is_null() const
{
    return padding == 0 && get_record() == nullptr;
}

void skr_resource_handle_t::reset()
{
    if (is_resolved())
        unload();
    std::memset(this, 0, sizeof(skr_resource_handle_t));
}

ESkrLoadingStatus skr_resource_handle_t::get_status(bool resolve) const
{
    if (is_null())
        return SKR_LOADING_STATUS_UNLOADED;
    if (padding != 0)
    {
        if(resolve)
        {
            auto system = skr::resource::GetResourceSystem();
            return system->GetResourceStatus(get_guid());
        }
        return SKR_LOADING_STATUS_UNLOADED;
    }
    auto record = get_record();
    return record->loadingStatus;
}

skr_resource_record_t* skr_resource_handle_t::get_record() const
{
    return (skr_resource_record_t*)(pointer & kResourceHandleRecordMask);
}

uint32_t skr_resource_handle_t::get_requester_id() const
{
    SKR_ASSERT(is_resolved());
    return requesterId;
}

ESkrRequesterType skr_resource_handle_t::get_requester_type() const
{
    SKR_ASSERT(is_resolved());
    return (ESkrRequesterType)(pointer & kResourceHandleRequesterTypeMask);
}

void skr_resource_handle_t::set_record(skr_resource_record_t* record)
{
    pointer = (uint64_t)record | (pointer & kResourceHandleRequesterTypeMask);
}

void skr_resource_handle_t::set_resolved(skr_resource_record_t* record, uint32_t inRequesterId, ESkrRequesterType requesterType)
{
    reset();
    pointer = (uint64_t)record | (uint64_t(requesterType) & kResourceHandleRequesterTypeMask);
    requesterId = inRequesterId;
}