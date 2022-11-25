#pragma once
#include "platform/configure.h"
#include "resource/resource_header.hpp"

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

typedef struct skr_vfs_t skr_vfs_t;
#if defined(__cplusplus)
    #include "binary/reader.h"
    #include "binary/writer.h"
namespace skr
{
namespace resource
{
/*
    resource load phase:
    Unloaded => request -> load binary -> deserialize => Loaded
    Loaded  => [requst & wait dependencies -> install/update install] => Installed
*/
struct RUNTIME_API SResourceFactory {
    virtual skr_type_id_t GetResourceType() = 0;
    virtual bool AsyncIO() { return true; }
    /*
        load factor range : [0, 100]
        0 means no async deserialize
        100 means one job per resource
        in between affect the number of jobs per frame, higher value means more jobs per frame
    */
    virtual float AsyncSerdeLoadFactor() { return 1.f; }
    virtual int Deserialize(skr_resource_record_t* record, skr_binary_reader_t* reader);
#ifdef SKR_RESOURCE_DEV_MODE
    virtual int DerserializeArtifacts(skr_resource_record_t* record, skr_binary_reader_t* reader) { return 0; };
#endif
    virtual bool Unload(skr_resource_record_t* record);
    virtual ESkrInstallStatus Install(skr_resource_record_t* record) { return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; }
    virtual bool Uninstall(skr_resource_record_t* record) { return true; }
    virtual ESkrInstallStatus UpdateInstall(skr_resource_record_t* record);
};
} // namespace resource
} // namespace skr
#endif