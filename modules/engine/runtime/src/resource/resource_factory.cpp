#include "SkrRT/resource/resource_factory.h"
#include "SkrRT/resource/resource_header.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrRTTR/type_registry.hpp"
#include "SkrRTTR/type.hpp"
#include "SkrCore/log.hpp"

namespace skr
{
namespace resource
{

bool SResourceFactory::Deserialize(skr_resource_record_t* record, SBinaryReader* reader)
{
    // TODO. resume rttr
    if (auto type = skr::rttr::get_type_from_guid(record->header.type))
    {
        auto p_obj = sakura_malloc_aligned(type->size(), type->alignment());
        // find & call ctor
        {
            auto ctor_data = type->record_data().find_ctor<void()>(
                skr::rttr::ETypeSignatureCompareFlag::Strict
            );
            auto ctor = static_cast<void(*)(void*)>(ctor_data->native_invoke);
            ctor(p_obj);
        }
        {
            using ReadBinProc = bool(void* o, void* r);
            auto read_bin_data = type->record_data().find_extern_method<ReadBinProc>(
                skr::rttr::SkrCoreExternMethods::ReadBin,
                rttr::ETypeSignatureCompareFlag::Strict
            ).value();
            auto read_bin = static_cast<ReadBinProc*>(read_bin_data->native_invoke);
            if (!read_bin(p_obj, reader))
            {
                // TODO: CALL DTOR IF FAILED
                SKR_UNIMPLEMENTED_FUNCTION();
                sakura_free_aligned(p_obj, type->alignment());
                p_obj = nullptr;
            }
        }
        record->resource = p_obj;
        return true;
    }
    SKR_LOG_FMT_ERROR(u8"Failed to deserialize resource of type {}", record->header.type);
    SKR_UNREACHABLE_CODE();
    return false;
}

bool SResourceFactory::Unload(skr_resource_record_t* record)
{
    record->header.dependencies.clear();
    if (record->destructor)
        record->destructor(record->resource);
#ifdef SKR_RESOURCE_DEV_MODE
    if (record->artifactsDestructor)
        record->artifactsDestructor(record->artifacts);
#endif
    return true;
}

ESkrInstallStatus SResourceFactory::UpdateInstall(skr_resource_record_t* record)
{
    SKR_UNREACHABLE_CODE();
    return SKR_INSTALL_STATUS_SUCCEED;
}
} // namespace resource
} // namespace skr