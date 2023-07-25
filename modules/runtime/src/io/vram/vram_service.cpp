#include "../../pch.hpp" // IWYU pragma: keep
#include "SkrRT/async/wait_timeout.hpp"
#include "../dstorage/dstorage_resolvers.hpp"

#include "vram_service.hpp"
#include "vram_resolvers.hpp"
#include "vram_readers.hpp"
#include "vram_batch.hpp"

namespace skr::io {

namespace VRAMUtils
{
inline static IOReaderId<IIORequestProcessor> CreateReader(VRAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
    auto reader = skr::SObjectPtr<VFSVRAMReader>::Create(service, desc->io_job_queue);
    return std::move(reader);
}

inline static IOReaderId<IIOBatchProcessor> CreateBatchReader(VRAMService* service, const skr_ram_io_service_desc_t* desc) SKR_NOEXCEPT
{
#ifdef _WIN32
    if (skr_query_dstorage_availability() == SKR_DSTORAGE_AVAILABILITY_HARDWARE)
    {
        auto reader = skr::SObjectPtr<DStorageVRAMReader>::Create(service);
        return std::move(reader);
    }
#endif
    return nullptr;
}
} // namespace RAMUtils

} // namespace skr::io