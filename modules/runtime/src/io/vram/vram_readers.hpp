#pragma once
#include "SkrRT/io/vram_io.hpp"
#include "vram_service.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {
struct VRAMService;

template<typename I = IIORequestProcessor>
struct VRAMReaderBase : public IIOReader<I>
{
    IO_RESOLVER_OBJECT_BODY
public:
    VRAMReaderBase(VRAMService* service) SKR_NOEXCEPT 
        : service(service) 
    {
        init_counters();
    }
    virtual ~VRAMReaderBase() SKR_NOEXCEPT {}

    void awakeService()
    {
        service->runner.awake();
    }

protected:
    VRAMService* service = nullptr;
};
} // namespace io
} // namespace skr