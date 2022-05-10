#pragma once
#include "platform/configure.h"
#include "resource/resource_header.h"
typedef enum ESkrInstallStatus
{
    SKR_INSTALL_STATUS_INPROGRESS,
    SKR_INSTALL_STATUS_SUCCEED,
    SKR_INSTALL_STATUS_FAILED,
} ESkrInstallStatus;
typedef enum ESkrLoadStatus
{
    SKR_LOAD_STATUS_INPROGRESS,
    SKR_LOAD_STATUS_SUCCEED,
    SKR_LOAD_STATUS_FAILED,
} ESkrLoadStatus;

#if defined(__cplusplus)
    #include "utils/serialize.hpp"
    #include "bitsery/serializer.h"
namespace skr
{
namespace resource
{
using SBinaryDeserializer = bitsery::Deserializer<bitsery::InputBufferAdapter<eastl::vector<uint8_t>>>;
using SBinarySerializer = bitsery::Serializer<bitsery::OutputBufferAdapter<eastl::vector<uint8_t>>>;
struct SBinaryArchive {
    SBinaryDeserializer* deserializer;
    SBinarySerializer* serializer;
};
/*
    resource load phase:
    Unloaded => request -> load binary -> deserialize => Loaded
    Loaded  => [requst & wait dependencies -> install/update install] => Installed
*/
struct RUNTIME_API SResourceFactory {
    virtual skr_type_id_t GetResourceType() = 0;
    virtual ESkrLoadStatus Load(skr_resource_record_t* record);
    virtual ESkrLoadStatus UpdateLoad(skr_resource_record_t* record);
    virtual bool Unload(skr_resource_record_t* record);
    virtual ESkrInstallStatus Install(skr_resource_record_t* record) { return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; }
    virtual bool Uninstall(skr_resource_record_t* record) { return true; }
    virtual ESkrInstallStatus UpdateInstall(skr_resource_record_t* record);
    virtual void DestroyResource(skr_resource_record_t* record);
};
} // namespace resource
} // namespace skr
#endif