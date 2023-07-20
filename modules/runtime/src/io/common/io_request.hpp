#pragma once
#include "../components/status_component.hpp"
#include "../components/file_component.hpp"

#include "pool.hpp"
#include <tuple>
#include <string.h> // ::strlen

namespace skr {
namespace io {

template<typename T>
[[nodiscard]] const T* get_component(const IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(IORequestComponentTID<T>::Get()))
        return static_cast<const T*>(c);
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

template<typename T>
[[nodiscard]] T* get_component(IIORequest* rq) SKR_NOEXCEPT
{
    if (auto c = rq->get_component(IORequestComponentTID<T>::Get()))
        return static_cast<T*>(c);
    SKR_UNREACHABLE_CODE();
    return nullptr;
}

template <typename Interface, typename...Components>
struct IORequestCRTP : public Interface
{
    IO_RC_OBJECT_BODY
public:
    IORequestCRTP(ISmartPoolPtr<Interface> pool) 
        : components(std::make_tuple(Components(this)...)), pool(pool)
    {

    }
    virtual ~IORequestCRTP() = default;

    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT 
    {
        get_component<IOFileComponent>(this)->set_vfs(_vfs);
    }

    void set_path(const char8_t* p) SKR_NOEXCEPT 
    { 
        get_component<IOFileComponent>(this)->set_path(p); 
    }

    [[nodiscard]] const char8_t* get_path() const SKR_NOEXCEPT 
    { 
        return get_component<IOFileComponent>(this)->get_path(); 
    }

    void use_async_complete() SKR_NOEXCEPT 
    { 
        get_component<IOStatusComponent>(this)->use_async_complete(); 
    }

    void use_async_cancel() SKR_NOEXCEPT 
    { 
        get_component<IOStatusComponent>(this)->use_async_cancel(); 
    }

    const skr_io_future_t* get_future() const SKR_NOEXCEPT 
    { 
        return get_component<IOStatusComponent>(this)->get_future(); 
    }

    void add_callback(ESkrIOStage stage, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        get_component<IOStatusComponent>(this)->add_callback(stage, callback, data);
    }
    
    void add_finish_callback(ESkrIOFinishPoint point, IOCallback callback, void* data) SKR_NOEXCEPT
    {
        get_component<IOStatusComponent>(this)->add_finish_callback(point, callback, data);
    }

    [[nodiscard]] virtual const IORequestComponent* get_component(skr_guid_t tid) const SKR_NOEXCEPT
    {
        return std::apply([&](const auto&... args) {
            const IORequestComponent* cs[] = { &args... };
            const skr_guid_t ids[] = { args.get_tid()... };
            for (uint64_t i = 0; i < sizeof...(Components); ++i)
            {
                if (ids[i] == tid)
                    return cs[i];
            }
            SKR_UNREACHABLE_CODE();
            return cs[0];
        }, components);
    }
    [[nodiscard]] virtual IORequestComponent* get_component(skr_guid_t tid) SKR_NOEXCEPT
    {
        return std::apply([&](auto&... args) {
            IORequestComponent* cs[] = { &args... };
            const skr_guid_t ids[] = { args.get_tid()... };
            for (uint64_t i = 0; i < sizeof...(Components); ++i)
            {
                if (ids[i] == tid)
                    return cs[i];
            }
            SKR_UNREACHABLE_CODE();
            return cs[0];
        }, components);
    }

private:
    std::tuple<Components...> components;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IORequestCRTP*>(ptr);
            p->pool->deallocate(p); 
        };
    }
protected:
    ISmartPoolPtr<Interface> pool = nullptr;
};

using RQPtr = skr::SObjectPtr<IIORequest>;
using IORequestQueue = IOConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

} // namespace io
} // namespace skr