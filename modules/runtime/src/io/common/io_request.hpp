#pragma once
#include "../components/status_component.hpp"
#include "../components/src_components.hpp"
#include "../components/blocks_component.hpp"
#include "SkrRT/platform/guid.hpp"
#include "SkrRT/containers/hashmap.hpp"

#include <tuple>
#include "pool.hpp"
#include "SkrProfile/profile.h"

namespace skr {
namespace io {

template<typename T>
[[nodiscard]] FORCEINLINE const T* io_component(const IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(CID<T>::Get()))
        return static_cast<const T*>(c);
    return (T*)nullptr;
}

template<typename T>
[[nodiscard]] FORCEINLINE T* io_component(IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(CID<T>::Get()))
        return static_cast<T*>(c);
    return (T*)nullptr;
}

template <typename Interface, typename...Components>
struct IORequestMixin : public Interface
{
    IO_RC_OBJECT_BODY
public:
    IORequestMixin(ISmartPoolPtr<Interface> pool, IIOService* service) 
        : service(service), components(std::make_tuple(Components(this)...)), pool(pool)
    {

    }
    virtual ~IORequestMixin() SKR_NOEXCEPT = default;

    [[nodiscard]] virtual const IORequestComponent* get_component(skr_guid_t tid) const SKR_NOEXCEPT
    {
        SkrZoneScopedN("IORequestMixin::get_component");
        auto& map = acquire_cmap();
        auto&& iter = map.find(tid);
        if (iter != map.end())
        {
            return dynamic_get(components, iter->second);
        }
        return nullptr;
    }

    [[nodiscard]] virtual IORequestComponent* get_component(skr_guid_t tid) SKR_NOEXCEPT
    {
        SkrZoneScopedN("IORequestMixin::get_component");
        auto& map = acquire_cmap();
        auto&& iter = map.find(tid);
        if (iter != map.end())
        {
            return dynamic_get(components, iter->second);
        }
        return nullptr;
    }

    IIOService* get_service() const SKR_NOEXCEPT
    {
        SKR_ASSERT(service && "service is null!");
        return service;
    }

    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IORequestMixin*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    
protected:
    IIOService* service = nullptr;
private:
    std::tuple<Components...> components;
    ISmartPoolPtr<Interface> pool = nullptr;

public:
    template <typename C>
    C* safe_comp() SKR_NOEXCEPT
    {
        auto c = io_component<C>(this);
        SKR_ASSERT(c && "failed to get component!");
        return c;
    }

    template <typename C>
    const C* safe_comp() const SKR_NOEXCEPT
    {
        auto c = io_component<C>(this);
        SKR_ASSERT(c && "failed to get component!");
        return c;
    }

    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT
    {
        safe_comp<PathSrcComponent>()->set_vfs(_vfs);
    }

    void set_path(const char8_t* p) SKR_NOEXCEPT 
    { 
        safe_comp<PathSrcComponent>()->set_path(p); 
    }

    [[nodiscard]] const char8_t* get_path() const SKR_NOEXCEPT 
    { 
        return safe_comp<PathSrcComponent>()->get_path(); 
    }

    void use_async_complete() SKR_NOEXCEPT 
    { 
        safe_comp<IOStatusComponent>()->use_async_complete(); 
    }

    void use_async_cancel() SKR_NOEXCEPT 
    { 
        safe_comp<IOStatusComponent>()->use_async_cancel(); 
    }

    const skr_io_future_t* get_future() const SKR_NOEXCEPT 
    { 
        return safe_comp<IOStatusComponent>()->get_future(); 
    }

    void add_callback(ESkrIOStage stage, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        safe_comp<IOStatusComponent>()->add_callback(stage, callback, data);
    }
    
    void add_finish_callback(ESkrIOFinishPoint point, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        safe_comp<IOStatusComponent>()->add_finish_callback(point, callback, data);
    }

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT 
    { 
        return safe_comp<BlocksComponent>()->get_blocks(); 
    }

    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT 
    { 
        safe_comp<BlocksComponent>()->add_block(block); 
    }

    void reset_blocks() SKR_NOEXCEPT 
    { 
        safe_comp<BlocksComponent>()->reset_blocks(); 
    }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT 
    { 
        return safe_comp<CompressedBlocksComponent>()->get_compressed_blocks(); 
    }

    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT
    {
        safe_comp<CompressedBlocksComponent>()->add_compressed_block(block); 
    }

    void reset_compressed_blocks() SKR_NOEXCEPT
    {
        safe_comp<CompressedBlocksComponent>()->reset_compressed_blocks(); 
    }

private:
    auto& acquire_cmap() const SKR_NOEXCEPT
    {
        static bool initialized = false;
        static skr::flat_hash_map<skr_guid_t, uint32_t, skr::guid::hash> map = {};
        if (!initialized)
        {
            std::apply([&](const auto&... args) {
                uint32_t i = 0;
                (map.emplace(args, i++), ...);
            }, std::make_tuple(CID<Components>::Get()...));
            initialized = true;
        }
        return map;
    }

    template <size_t n, typename... T>
    FORCEINLINE auto dynamic_get_impl(std::tuple<T...>& tpl, uint64_t i)
    {
        if (i == n)
            return static_cast<IORequestComponent*>(&std::get<n>(tpl));
        else if (n == sizeof...(T) - 1)
        {
            SKR_ASSERT(0 && "Tuple element out of range.");
            return (IORequestComponent*)nullptr;
        }
        else
            return dynamic_get_impl<(n < sizeof...(T)-1 ? n+1 : 0)>(tpl, i);
    }

    template <typename... T>
    FORCEINLINE auto dynamic_get(std::tuple<T...>& tpl, uint64_t i)
    {
        return dynamic_get_impl<0>(tpl, i);
    }

    template <size_t n, typename... T>
    FORCEINLINE auto dynamic_get_impl(const std::tuple<T...>& tpl, uint64_t i) const
    {
        if (i == n)
            return static_cast<const IORequestComponent*>(&std::get<n>(tpl));
        else if (n == sizeof...(T) - 1)
        {
            SKR_ASSERT(0 && "Tuple element out of range.");
            return (const IORequestComponent*)nullptr;
        }
        else
            return dynamic_get_impl<(n < sizeof...(T)-1 ? n+1 : 0)>(tpl, i);
    }

    template <typename... T>
    FORCEINLINE auto dynamic_get(const std::tuple<T...>& tpl, uint64_t i) const
    {
        return dynamic_get_impl<0>(tpl, i);
    }
};

using IORequestQueue = IOConcurrentQueue<IORequestId>;  
using IORequestArray = skr::vector<IORequestId>;

} // namespace io
} // namespace skr