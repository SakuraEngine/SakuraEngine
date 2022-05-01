#pragma once
#include "platform/configure.h"
#include "resource/resource_header.h"
typedef enum ESkrInstallStatus
{
    SKR_INSTALL_STATUS_INPROGRESS,
    SKR_INSTALL_STATUS_SUCCEED,
    SKR_INSTALL_STATUS_FAILED,
} ESkrInstallStatus;

#if defined(__cplusplus)
    #include "utils/serialize.hpp"
namespace skr
{
namespace resource
{
using SBinaryArchive = bitsery::Deserializer<bitsery::InputBufferAdapter<eastl::vector<uint8_t>>>;
/*
    resource load phase:
    Unloaded => request -> load binary -> deserialize header -> deserialize => Loaded
    Loaded  => requst & wait dependencies -> install/update install => Installed
*/
struct SResourceFactory {
    virtual skr_type_id_t GetResourceType() = 0;
    virtual bool Deserialize(skr_resource_record_t* record, SBinaryArchive& archive);
    virtual bool Unload(skr_resource_record_t* record);
    virtual ESkrInstallStatus Install(skr_resource_record_t* record) { return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; }
    virtual bool Uninstall(skr_resource_record_t* record) { return true; }
    virtual ESkrInstallStatus UpdateInstall(skr_resource_record_t* record);
    virtual void DestroyResource(skr_resource_record_t* record);
};
} // namespace resource
} // namespace skr
#endif