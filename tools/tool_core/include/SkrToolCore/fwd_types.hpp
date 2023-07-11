#pragma once
#include "SkrToolCore/module.configure.h"
#include "SkrRT/platform/configure.h"

struct skr_vfs_t;
struct SkrToolCoreModule;
SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service);
struct skr_io_vram_service_t;

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