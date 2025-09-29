#include "SkrRuntime/resource/resource_header.hpp"
#include "SkrRuntime/resource/resource_system.hpp"

namespace skr
{
void Serialize<SResourceHeader>::read(ArchiveRead& r, SResourceHeader& v)
{
    Archive::ObjectScope obj_scope(r);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    uint32_t function = 0;

    SKR_FAST_CHECK(r.key_value(u8"function", function), );
    SKR_FAST_CHECK(r.key_value(u8"version", v.version), );
    SKR_FAST_CHECK(r.key_value(u8"guid", v.guid), );
    SKR_FAST_CHECK(r.key_value(u8"type", v.type), );

    SKR_FAST_CHECK(r.key(u8"dependencies"), );
    {
        Archive::ArrayScope arr_scope(r);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // read dependencies count
        uint64_t dependencies_size = 0;
        SKR_FAST_CHECK(r.array_size<uint64_t>(dependencies_size), );
        v.dependencies.resize_unsafe((size_t)dependencies_size);

        // read each dependency guid
        for (uint64_t i = 0; i < dependencies_size; i++)
        {
            SKR_FAST_CHECK(r.value(v.dependencies[i]), );
        }
    }
}
void Serialize<SResourceHeader>::write(ArchiveWrite& w, const SResourceHeader& v)
{
    Archive::ObjectScope obj_scope(w);
    SKR_FAST_CHECK(obj_scope.is_success(), );

    uint32_t function = 1;

    SKR_FAST_CHECK(w.key_value(u8"function", function), );
    SKR_FAST_CHECK(w.key_value(u8"version", v.version), );
    SKR_FAST_CHECK(w.key_value(u8"guid", v.guid), );
    SKR_FAST_CHECK(w.key_value(u8"type", v.type), );

    SKR_FAST_CHECK(w.key(u8"dependencies"), );
    {
        Archive::ArrayScope arr_scope(w);
        SKR_FAST_CHECK(arr_scope.is_success(), );

        // write dependencies count
        SKR_FAST_CHECK(w.array_size<uint64_t>((uint64_t)v.dependencies.size()), );

        // write each dependency guid
        for (const auto& dep : v.dependencies)
        {
            SKR_FAST_CHECK(w.value(dep), );
        }
    }
}
} // namespace skr

uint32_t SResourceRecord::AddReference()
{
    ++referenceCount;
    return referenceCount;
}

void SResourceRecord::RemoveReference()
{
    --referenceCount;
    if (referenceCount == 0)
    {
        auto system = skr::GetResourceSystem();
        system->UnloadResource(header.guid);
    }
}

bool SResourceRecord::IsReferenced() const
{
    return referenceCount > 0;
}

void SResourceRecord::SetStatus(EResourceLoadingStatus newStatus)
{
    if (newStatus != loadingStatus)
    {
        loadingStatus = newStatus;
    }
}