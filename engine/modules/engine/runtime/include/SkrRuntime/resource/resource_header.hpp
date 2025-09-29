#pragma once
#include "SkrOS/thread.h"
#include "SkrContainersDef/vector.hpp"
#include "SkrRuntime/resource/resource_handle.h"

namespace skr
{
struct ResourceRequest;
}
typedef struct SResourceRecord SResourceRecord;
#define TRACK_RESOURCE_REQUESTS 1

typedef struct SResourceHeader
{
    uint32_t version;
    skr::GUID guid;
    skr::GUID type;
    skr::Vector<SResourceHandle> dependencies;
} SResourceHeader;

enum class EResourceLoadingStatus : uint32_t
{
    Unloaded,
    Loading,
    Loaded,
    WaitingDependencies,
    Installing,
    Installed,
    Uninstalling,
    Unloading,
    Error,
    Count
};

struct SKR_RUNTIME_API SResourceRecord
{
    SResourceHeader header;
    std::atomic<uint32_t> referenceCount = 0;
    void* resource = nullptr;
    std::atomic<EResourceLoadingStatus> loadingStatus = EResourceLoadingStatus::Unloaded;
    skr::ResourceRequest* activeRequest;

    void SetStatus(EResourceLoadingStatus);
    uint32_t AddReference();
    void RemoveReference();
    bool IsReferenced() const;
};

namespace skr
{
template <>
struct SKR_RUNTIME_API Serialize<SResourceHeader>
{
    static void read(ArchiveRead& r, SResourceHeader& v);
    static void write(ArchiveWrite& w, const SResourceHeader& v);
};
}; // namespace skr