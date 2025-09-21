#pragma once
#include "SkrRuntime/config.h"
#include "SkrRuntime/resource/resource_header.hpp"

typedef enum ESkrInstallStatus
{
    SKR_INSTALL_STATUS_INPROGRESS,
    SKR_INSTALL_STATUS_SUCCEED,
    SKR_INSTALL_STATUS_FAILED,
} ESkrInstallStatus;

typedef struct skr_vfs_t skr_vfs_t;
#if defined(__cplusplus)
    #include "SkrBase/types.h"
    #include "SkrBase/types.h"
namespace skr
{
/*
    resource load phase:
    Unloaded => request -> load binary -> deserialize => Loaded
    Loaded  => [requst & wait dependencies -> install/update install] => Installed
*/
struct SKR_RUNTIME_API ResourceFactory {
    virtual skr_guid_t GetResourceType() = 0;
    virtual bool AsyncIO() { return true; }
    /*
        load factor range : [0, 100]
        0 means no async deserialize
        100 means one job per resource
        in between affect the number of jobs per frame, higher value means more jobs per frame
    */
    virtual float AsyncSerdeLoadFactor() { return 1.f; }
    virtual bool Deserialize(SResourceRecord* record, skr::ArchiveRead* reader);
#ifdef SKR_RESOURCE_DEV_MODE
    virtual bool DerserializeArtifacts(SResourceRecord* record, skr::ArchiveRead* reader) { return 0; };
#endif
    virtual bool Unload(SResourceRecord* record);
    virtual ESkrInstallStatus Install(SResourceRecord* record) { return ESkrInstallStatus::SKR_INSTALL_STATUS_SUCCEED; }
    virtual bool Uninstall(SResourceRecord* record) { return true; }
    virtual ESkrInstallStatus UpdateInstall(SResourceRecord* record);
};
} // namespace skr
#endif