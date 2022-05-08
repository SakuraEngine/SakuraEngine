#include "resource/resource_handle.h"
#include "platform/debug.h"
#include "platform/guid.h"
#include "platform/configure.h"
#include "resource/config_resource.h"
#include "resource/resource_header.h"
#include "resource/resource_system.h"

skr_resource_handle_t::skr_resource_handle_t()
{
    reset();
}

skr_resource_handle_t::skr_resource_handle_t(const skr_resource_handle_t& other)
{
    memcpy(this, &other, sizeof(skr_resource_handle_t));
}

skr_resource_handle_t::skr_resource_handle_t(skr_resource_handle_t&& other)
{
    memcpy(this, &other, sizeof(skr_resource_handle_t));
}

void skr_resource_handle_t::set_ptr(void* ptr)
{
    reset();
    auto system = skr::resource::GetResourceSystem();
    auto record = system->_GetRecord(ptr);
    if (!record)
        return;
    pointer = (int64_t)record;
}

void skr_resource_handle_t::set_guid(const skr_guid_t& inGUID)
{
    guid = inGUID;
    SKR_ASSERT(padding != 0 || is_null());
}

bool skr_resource_handle_t::is_resolved() const
{
    return padding == 0 && pointer != 0;
}

void* skr_resource_handle_t::get_ptr() const
{
    SKR_ASSERT(padding == 0);
    return pointer != 0 ? ((skr_resource_record_t*)pointer)->resource : nullptr;
}

skr_guid_t skr_resource_handle_t::get_guid() const
{
    SKR_ASSERT(padding != 0);
    return guid;
}

void* skr_resource_handle_t::get_resolved(bool requireInstalled) const
{
    if (is_null())
        return nullptr;
    auto record = ((skr_resource_record_t*)pointer);
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
    auto record = ((skr_resource_record_t*)pointer);
    SKR_ASSERT(record);
    return record->header.guid;
}

void skr_resource_handle_t::resolve(bool requireInstalled, uint32_t inRequester)
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
    {
        auto system = skr::resource::GetResourceSystem();
        system->LoadResource(guid, requireInstalled, inRequester);
        auto record = system->_GetRecord(guid);
        reset();
        if (record)
        {
            requester = inRequester;
            pointer = (int64_t)record;
        }
    }
}

void skr_resource_handle_t::serialize()
{
    SKR_ASSERT(!is_null());
    if (padding != 0)
        return;
    auto record = ((skr_resource_record_t*)pointer);
    guid = record->header.guid;
}

bool skr_resource_handle_t::is_null() const
{
    return padding == 0 && pointer == 0;
}

void skr_resource_handle_t::reset()
{
    std::memset(this, 0, sizeof(skr_resource_handle_t));
}

ESkrLoadingStatus skr_resource_handle_t::get_status()
{
    if (padding != 0 || is_null())
        return SKR_LOADING_STATUS_UNLOADED;
    auto record = ((skr_resource_record_t*)pointer);
    return record->loadingStatus;
}