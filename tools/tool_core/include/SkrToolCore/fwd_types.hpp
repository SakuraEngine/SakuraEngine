#pragma once
#include "SkrBase/config.h"
#include "SkrRT/config.h"

struct skr_vfs_t;
struct SkrToolCoreModule;
SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service);
SKR_DECLARE_TYPE_ID_FWD(skr::io, IVRAMService, skr_io_vram_service);

namespace skd sreflect
{
struct SProject;
namespace asset sreflect
{
struct SImporter;
struct SAssetRecord;
struct SCookSystem;
struct SCooker;
struct SCookContext;
}
}