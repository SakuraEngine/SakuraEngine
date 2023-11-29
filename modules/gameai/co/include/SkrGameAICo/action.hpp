#pragma once
#include "SkrGameAICo/module.configure.h"

#if __cpp_impl_coroutine
#include <atomic>
#include "SkrRT/containers/vector.hpp"
#include "SkrRT/containers/optional.hpp"
#include <queue>

#if __cpp_lib_coroutine
    #include <coroutine>
#else
    #include <experimental/coroutine>
    //UB
    namespace std
    {
        template<class T>
        using coroutine_handle = experimental::coroutine_handle<T>;
        using suspend_always = experimental::suspend_always;
        using suspend_never = experimental::suspend_never;
    }
#endif

namespace skr
{
namespace gameai
{
    enum class coaction_state
    {
        running,
        succeed,
        failed,
        interupted,
    };

    struct coaction_promise_base_t
    {
        // Keep a coroutine handle referring to the parent coroutine if any. That is, if we
        // co_await a coroutine within another coroutine, this handle will be used to continue
        // working from where we left off.
        std::coroutine_handle<> parent;
        skr::vector<std::coroutine_handle<>> children;
        //pending parallel actions
        int* counter = nullptr;
        bool racing = false;
        coaction_state state = coaction_state::interupted;
        std::suspend_always initial_suspend() { return {}; }
        auto final_suspend() noexcept { 
            struct awaiter {
                bool await_ready() const noexcept { return false; }

                void await_resume() const noexcept {}

                std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) noexcept {
                    auto h = std::coroutine_handle<coaction_promise_base_t>::from_address(handle.address());
                    auto& p = h.promise();
                    auto parent = p.parent;
                    auto counter = p.counter;
                    bool win = p.racing && p.state == coaction_state::succeed;
                    if (!win && (counter && *counter > 0))
                    {
                        --(*counter);
                        return std::noop_coroutine();
                    }
                    else if (parent)
                    {
                        std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.clear();
                        return parent;
                    }
                    return std::noop_coroutine();
                }
            };
            return awaiter{};
        }
    };

    template<class T>
    struct action_result_t
    {
        coaction_state state = coaction_state::interupted;
        skr::optional<T> result;

        void set_result(T value) noexcept
        {
            result = std::move(value);
            state = coaction_state::succeed;
        }

        void set_failed() noexcept
        {
            state = coaction_state::failed;
        }

        void set_interupted() noexcept
        {
            state = coaction_state::interupted;
        }
    };

    //action with return value
    template<class T = void>
    struct coaction
    {
        struct promise_type : public coaction_promise_base_t
        {
            coaction get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
            void return_value(T value) noexcept { result.set_result(std::move(value)); }
            void return_value(nullptr_t) noexcept { result.set_failed(); }
            void unhandled_exception() noexcept { std::terminate(); }
            skr::optional<T> result;
        };
        using handle_t = std::coroutine_handle<promise_type>;
        coaction() : handle(nullptr) {}
        coaction(handle_t handle) : handle(handle) {}
        coaction(coaction&& rhs) noexcept : handle(rhs.handle) { rhs.handle = nullptr; }
        ~coaction() { if(handle) handle.destroy(); }
        auto get() const noexcept { return action_result_t<T>{state(), std::move(handle.promise().result)}; }
        auto state() const noexcept { return handle.promise().result.state; }
        handle_t handle;

        bool await_ready() const noexcept { return handle.done(); }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> parent) const noexcept 
        {
            handle.promise().parent = parent; 
            std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.push_back(handle);
            return handle;
        }
        T await_resume() const noexcept { return get(); }
    };

    inline struct succeed_t{} succeed;

    //action without return value
    template<>
    struct coaction<void>
    {
        struct promise_type : public coaction_promise_base_t
        {
            coaction get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
            void return_value(nullptr_t) noexcept { state = coaction_state::failed; }
            void return_value(succeed_t) noexcept { state = coaction_state::succeed; }
            void unhandled_exception() noexcept { std::terminate(); }
        };
        using handle_t = std::coroutine_handle<promise_type>;
        coaction() : handle(nullptr) {}
        coaction(handle_t handle) : handle(handle) {}
        coaction(coaction&& rhs) noexcept : handle(rhs.handle) { rhs.handle = nullptr; }
        ~coaction() { if(handle) handle.destroy(); }
        coaction_state get() const noexcept { return handle.promise().state; }
        coaction_state state() const noexcept { return handle.promise().state; }
        handle_t handle;

        bool resume() noexcept
        {
            if(handle.done())
                return true;
            handle.resume();
            return handle.done();
        }

        bool await_ready() const noexcept { return handle.done(); }
        std::coroutine_handle<> await_suspend(std::coroutine_handle<> parent) const noexcept 
        { 
            handle.promise().parent = parent; 
            std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.push_back(handle);
            return handle;
        }
        coaction_state await_resume() const noexcept { return handle.promise().state; }
    };

    struct cocontext_t
    {
        struct FrameAwaitable
        {
            bool await_ready() const noexcept { return false; }
            SKR_GAMEAI_CO_API void await_suspend(std::coroutine_handle<> handle) const noexcept;
            double await_resume() const noexcept { return ctx->deltaTime; }

            cocontext_t* ctx;
        };

        struct TimerAwaitable
        {
            bool await_ready() const noexcept { return false; }
            SKR_GAMEAI_CO_API void await_suspend(std::coroutine_handle<> handle) const noexcept;
            void await_resume() const noexcept {}

            cocontext_t* ctx;
            double time;
        };

        struct TimeWaiter
        {
            std::coroutine_handle<> handle;
            double time;
            bool operator<(const TimeWaiter& rhs) const noexcept { return time < rhs.time; }
        };
        
        FrameAwaitable next_frame() noexcept { return {this}; }
        TimerAwaitable wait(double time) noexcept { return {this, this->time + time}; }

        SKR_GAMEAI_CO_API void interupt_single(std::coroutine_handle<> handle) noexcept;
        SKR_GAMEAI_CO_API void interupt_tree(std::coroutine_handle<> handle) noexcept;

        skr::vector<std::coroutine_handle<>> pendingFrameWaiters;
        std::priority_queue<TimeWaiter> pendingTimeWaiters;

        double time = 0.0;
        double deltaTime = 0.0;

        SKR_GAMEAI_CO_API void update(double time, bool nextFrame = true) noexcept;
    };

    template<class... Ts>
    auto parallel(Ts... actions)
    {
        struct awaiter
        {
            bool await_ready() const noexcept 
            { 
                auto resume = [&](auto& a) 
                { 
                    if(!a.await_ready()) a.handle.resume(); 
                    return a.await_ready(); 
                };
                counter = std::apply([](auto&&... args) { return (!resume(args) + ... + 0); }, actions); 
                return counter == 0;
            }
            void await_suspend(std::coroutine_handle<> parent) const noexcept
            {
                auto set = [&](auto& a) 
                { 
                    a.handle.promise().counter = &counter; 
                    a.handle.promise().parent = parent; 
                    std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.push_back(a.handle);
                };
                std::apply([&](auto&&... args) { (set(args), ...); }, actions);
            }
            auto await_resume() const noexcept 
            {
                std::coroutine_handle<> parent = std::get<0>(actions).handle.promise().parent;
                std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.clear();
                auto get = [&](auto& a) { return a.get(); };
                return std::apply([&](auto&&... args) { return std::make_tuple(get(args)...); }, actions);
            }
            std::tuple<Ts...> actions;
            mutable int counter = 0;
        };
        return awaiter{std::tuple<Ts...>{std::move(actions)...}};
    }


    template<class... Ts>
    auto race(cocontext_t& ctx, Ts... actions)
    {
        struct awaiter
        {
            cocontext_t& ctx;
            std::tuple<Ts...> actions;
            bool await_ready() const noexcept 
            { 
                auto resume = [&](auto& a) { if(!a.await_ready()) a.handle.resume(); return a.state() == coaction_state::succeed; };
                bool ready = std::apply([&](auto&&... args) { return (resume(args) || ...); }, actions); 
                if(!ready)
                {
                    auto count = [&](auto& a) { return a.await_ready(); };
                    counter = std::apply([&](auto&&... args) { return (!count(args) + ... + 0); }, actions);
                    return false;
                }
                return true;
            }
            void await_suspend(std::coroutine_handle<> handle) const noexcept
            {
                auto set = [&](auto& a) 
                { 
                    auto& promise = a.handle.promise();
                    promise.parent = handle; 
                    promise.racing = true;
                    promise.counter = &counter;
                    std::coroutine_handle<coaction_promise_base_t>::from_address(handle.address()).promise().children.push_back(a.handle);
                };
                std::apply([&](auto&&... args) { (set(args), ...); }, actions);
            }
            auto await_resume() const noexcept 
            {
                std::coroutine_handle<> parent = std::get<0>(actions).handle.promise().parent;
                std::coroutine_handle<coaction_promise_base_t>::from_address(parent.address()).promise().children.clear();
                auto get = [&](auto& a) { ctx.interupt_tree(a.handle); return a.get(); };
                return std::apply([&](auto&&... args) { return std::make_tuple(get(args)...); }, actions);
            }
            mutable int counter = 0;
        };
        return awaiter{ctx, std::tuple<Ts...>{std::move(actions)...}};
    }
}
}
#endif