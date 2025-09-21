#pragma once
#include <SkrBase/config.h>
#include <SkrBase/meta.h>

struct skr_vfs_t;
struct SkrToolCoreModule;
SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service);
SKR_DECLARE_TYPE_ID_FWD(skr::io, IVRAMService, skr_io_vram_service);

namespace skr::task
{
    struct event_t;
}

namespace skd
{
struct SProject;
namespace asset
{
struct Importer;
struct AssetMetaFile;
struct CookSystem;
struct Cooker;
struct CookContext;
} // namespace asset
} // namespace skd