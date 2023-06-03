#include "io_runnner.hpp"
#include "io_resolver.hpp"

namespace skr {
namespace io {
    
struct OpenFileResolver : public IOBatchResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        request->open_file(); 
    }
};

IOBatchResolverId IIOService::create_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<OpenFileResolver>::Create();
}

} // namespace io
} // namespace skr