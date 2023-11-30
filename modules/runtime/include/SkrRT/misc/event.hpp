#pragma once
#include "SkrRT/containers/concurrent_queue.h"
#include "SkrRT/containers/hashmap.hpp"
#include "SkrRT/containers_new/vector.hpp"
#include "SkrRT/containers/SPtr.hpp"
#include "SkrRT/containers/function.hpp"
#include "SkrRT/containers/function_ref.hpp"

namespace skr
{
    struct EventConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
    {
        static constexpr const char* kEventQueueName = "Event";
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static inline void* malloc(size_t size) { return sakura_mallocN(size, kEventQueueName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kEventQueueName); }
    };
    template<class T>
    struct EventRegistry;
    template<class... Args>
    struct EventRegistry<void(Args...)>
    {
        flat_hash_map<void*, void(*)(void*, Args...)> methods;
        flat_hash_map<SWeakPtr<void>, void(*)(void*, Args...)> weakMethods;
        flat_hash_map<void*, skr::function<void(Args...)>> methodLambdas;
        flat_hash_map<SWeakPtr<void>, skr::function<void(Args...)>> weakMethodLambdas;
        flat_hash_map<int, skr::function<void(Args...)>> lambdas;
        int currentHandle = 0;

        void Clear()
        {
            methods.clear();
            weakMethods.clear();
            methodLambdas.clear();
            weakMethodLambdas.clear();
            lambdas.clear();
            currentHandle = 0;
        }

        template<class O, auto Method>
        void AddMethod(O* obj)
        {
            methods[obj] = +[](void* u, Args... args) { (static_cast<O*>(u)->*Method)(args...); };
        }

        template<class O, auto Method>
        void AddWeakMethod(SPtr<O> obj)
        {
            weakMethods[obj] = +[](void* u, Args... args) { (static_cast<O*>(u)->*Method)(args...); };
        }

        template<class O, class F>
        void AddMethodLambda(O* obj, F&& lambda)
        {
            methodLambdas[obj] = std::forward<F>(lambda);
        }

        template<class O, class F>
        void AddWeakMethodLambda(SPtr<O> obj, F&& lambda)
        {
            weakMethodLambdas[obj] = std::forward<F>(lambda);
        }

        template<class F>
        int AddLambda(F&& lambda)
        {
            lambdas[currentHandle] = std::forward<F>(lambda);
            ++currentHandle;
            return currentHandle - 1;
        }

        void Remove(int handle)
        {
            lambdas.erase(handle);
        }

        void Remove(void* obj)
        {
            methods.erase(obj);
            methodLambdas.erase(obj);
        }

        void Remove(SPtr<void> obj)
        {
            weakMethods.erase(obj);
            weakMethodLambdas.erase(obj);
        }
    };
    template<class T>
    struct Event;
    template<class... Args>
    struct Event<void(Args...)> : public EventRegistry<void(Args...)>
    {
        template<class... Ts>
        void Broadcast(Ts&&... e)
        {
            for (auto& [obj, method] : this->methods)
            {
                method(obj, std::forward<Ts>(e)...);
            }
            for(auto iter = this->weakMethods.begin(); iter != this->weakMethods.end();)
            {
                if (auto ptr = iter->first.lock())
                {
                    iter->second(ptr.get(), std::forward<Ts>(e)...);
                    ++iter;
                }
                else
                {
                    iter = this->weakMethods.erase(iter);
                }
            }
            for (auto& [obj, method] : this->methodLambdas)
            {
                method(std::forward<Ts>(e)...);
            }
            for(auto iter = this->weakMethodLambdas.begin(); iter != this->weakMethodLambdas.end();)
            {
                if (auto ptr = iter->first.lock())
                {
                    iter->second(std::forward<Ts>(e)...);
                    ++iter;
                }
                else
                {
                    iter = this->weakMethodLambdas.erase(iter);
                }
            }
            for (auto& [handle, lambda] : this->lambdas)
            {
                lambda(std::forward<Ts>(e)...);
            }
        }
    };
    template<class T>
    struct EventQueue;
    template<class... Args>
    struct EventQueue<void(Args...)> : public EventRegistry<void(Args...)>
    {
        using T = std::tuple<Args...>;
        ConcurrentQueue<T, EventConcurrentQueueTraits> queue;
        template<class... Ts>
        void Enqueue(Ts&&... e)
        {
            queue.enqueue(T{std::forward<Ts>(e)...});
        }

        void Invoke()
        {
            T e;
            while (queue.try_dequeue(e))
            {
                for (auto& [obj, method] : this->methods)
                {
                    std::apply(method, std::tuple_cat(std::make_tuple(obj), e));
                }
                for(auto iter = this->weakMethods.begin(); iter != this->weakMethods.end();)
                {
                    if (auto ptr = iter->first.lock())
                    {
                        std::apply(iter->second, std::tuple_cat(std::make_tuple(ptr.get()), e));
                        ++iter;
                    }
                    else
                    {
                        iter = this->weakMethods.erase(iter);
                    }
                }
                
                for (auto& [obj, method] : this->methodLambdas)
                {
                    std::apply(method, e);
                }
                for (auto iter = this->weakMethodLambdas.begin(); iter != this->weakMethodLambdas.end();)
                {
                    if (auto ptr = iter->first.lock())
                    {
                        std::apply(iter->second, e);
                        ++iter;
                    }
                    else
                    {
                        iter = this->weakMethodLambdas.erase(iter);
                    }
                }
                for (auto& [handle, lambda] : this->lambdas)
                {
                    std::apply(lambda, e);
                }
            }
        }
    };
}