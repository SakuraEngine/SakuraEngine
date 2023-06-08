#include "../common/io_resolver.hpp"
#include "../common/io_request.hpp"

namespace skr {
namespace io {

struct DStorageFileResolver : public IORequestResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        auto rq = skr::static_pointer_cast<IORequestBase>(request);
    }
};

IORequestResolverId create_dstorage_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<DStorageFileResolver>::Create();
}

} // namespace io
} // namespace skr