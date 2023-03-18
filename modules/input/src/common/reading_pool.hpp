#pragma once
#include "common_layer.hpp"
#include "platform/memory.h"
#include "utils/log.h"
#include <utils/concurrent_queue.h>

namespace skr {
namespace input {

struct SKR_INPUT_API CommonInputReadingPoolBase : public CommonInputReadingProxy
{
    CommonInputReadingPoolBase() SKR_NOEXCEPT {}
    virtual ~CommonInputReadingPoolBase() SKR_NOEXCEPT;
    CommonInputReadingPoolBase(const CommonInputReadingPoolBase&) = delete;
    CommonInputReadingPoolBase& operator=(const CommonInputReadingPoolBase&) = delete;
    CommonInputReadingPoolBase(CommonInputReadingPoolBase&&) = delete;
    CommonInputReadingPoolBase& operator=(CommonInputReadingPoolBase&&) = delete;

    void cleanup(uint64_t Now, uint64_t LifetimeUSec) SKR_NOEXCEPT
    {

    }

    void ReportLeaking()
    {
        // SKR_LOG_INFO("CommonInputReadingPool::~CommonInputReadingPool() - %llu objects leaked", count);
    }
protected:

    void release(CommonInputReading* ptr) SKR_NOEXCEPT final
    {
        // SKR_LOG_INFO("CommonInputReadingPool::release() - releasing object");
        ptr->~CommonInputReading();
        m_pool.enqueue(ptr);
        count--;
    }
    moodycamel::ConcurrentQueue<CommonInputReading*> m_pool;
    uint64_t count = 0;
};

template<typename T>
struct CommonInputReadingPool : public CommonInputReadingPoolBase
{
    CommonInputReadingPool() : CommonInputReadingPoolBase() {}
    CommonInputReadingPool(const CommonInputReadingPool&) = delete;
    CommonInputReadingPool& operator=(const CommonInputReadingPool&) = delete;
    CommonInputReadingPool(CommonInputReadingPool&&) = delete;
    CommonInputReadingPool& operator=(CommonInputReadingPool&&) = delete;

    template<typename... Args>
    T* acquire(CommonInputReadingProxy* pPool, struct CommonInputDevice* pDevice, Args&&... args)
    {
        CommonInputReading* ptr = nullptr;
        if (!m_pool.try_dequeue(ptr))
        {
            // SKR_LOG_INFO("CommonInputReadingPool::acquire() - creating new object");
            ptr = SkrNew<T>(pPool, pDevice, std::forward<Args>(args)...);
        }
        else
        {
            // SKR_LOG_INFO("CommonInputReadingPool::acquire() - reallocating object");
        }
        count++;
        new (ptr) T (pPool, pDevice, std::forward<Args>(args)...);
        ptr->add_ref();
        return static_cast<T*>(ptr);
    }
};

} }