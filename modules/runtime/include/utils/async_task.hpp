#pragma once
#include "platform/configure.h"
#include <type_traits>
#include "platform/atomic.h"
#include "platform/memory.h"

namespace skr
{

struct AsyncTaskException 
{
    enum class eEx : uint32_t
    {
        TaskIsAlreadyRunning,
        TaskIsAlreadyFinished
    };

    AsyncTaskException() = default;
    AsyncTaskException(eEx e)
        : e(e)
    {
    }

    eEx e;
};

enum class FutureStatus : uint32_t
{
    Ready,
    Timeout,
    Deferred
};

template<typename Result>
struct IFuture
{

    virtual bool valid() const = 0;
    virtual void wait() = 0;
    virtual void wait_for(uint32_t ms) = 0;
    virtual void catch_and_free_exception(AsyncTaskException* e) = 0;
    virtual Result get() = 0;
};

// AsyncTask
// Asynchronous task progress handler class
//  - Asynchronous worker task should be defined into the do_in_background(),
//  - Feedback system elements should be handled by the on_pre_execute()/onProgressUpdate()/on_post_execute()/on_cancelled()
//  - execute() start the do_in_background()
//  - Refresh the feedback by the on_callback_loop()
// Nocopy object. on_callback_loop() and get() could rethrow the do_in_background() thrown exceptions.
template <typename Launcher, typename Progress, typename Result, typename... Params>
class AsyncTaskBase
{
public:
    enum class Status : uint32_t
    {
        PENDING, // Indicates that the task has not been executed yet.
        RUNNING, // Indicates that the task is running.
        FINISHED // Indicates that the task is finished.
    };

private:
    Status status = Status::PENDING;
    // Result handling
    Result result{};
    // Cancellation handling
    SAtomic32 cancelled{};
    IFuture<Result>* future = nullptr;

public:
    AsyncTaskBase() = default;

protected:
    AsyncTaskBase(AsyncTaskBase const&) = delete;
    AsyncTaskBase(AsyncTaskBase&&) = delete;
    AsyncTaskBase& operator=(AsyncTaskBase const&) = delete;
    AsyncTaskBase& operator=(AsyncTaskBase&&) = delete;
    virtual ~AsyncTaskBase() noexcept
    {
        auto const status = get_status();
        if (status != Status::RUNNING)
            return;

        if (!this->is_cancelled())
            this->cancel();

        if (!this->future->valid())
            return;

        this->future->wait();
    };

public:
    // Initiate the asynchronous task
    // If the task is already began, AsyncTaskIllegalStateException will be thrown
    // @MainThread
    AsyncTaskBase<Progress, Result, Params...>& execute(Params const&... params) noexcept(false)
    {
        AsyncTaskException* exception = nullptr;
        switch (status)
        {
            case Status::PENDING:
                break; // Everything is ok.
            case Status::RUNNING:
                exception = SkrNew<AsyncTaskException>(AsyncTaskException::eEx::TaskIsAlreadyRunning);
            case Status::FINISHED:
                exception = SkrNew<AsyncTaskException>(AsyncTaskException::eEx::TaskIsAlreadyFinished);
        }
        if (exception)
        {
            this->on_exception(exception);
            future->catch_and_free_exception(exception);
        }

        this->status = Status::RUNNING;
        this->on_pre_execute();
        this->future = Launcher::async(
            [this](Params const&... params) -> Result {
                if (is_cancelled())
                    return {}; // Protect against undefined behavoiur, if Dtor is invoked before the - pure virtual function represented - task would be started

                try
                {
                    return this->post_result(this->do_in_background(params...));
                } catch (...)
                {
                    cancel();
                }
                return {};
            },
            params...);

        return *this;
    }

    // @MainThread
    Status get_status() const noexcept { return status; }

protected:
    // Background worker task
    // Exception can be thrown, it will be rethrown in get(), on_callback_loop() or Dtor()
    // @WorkerThread
    virtual Result do_in_background(Params const&... params) = 0;

    // Define the store mechanism of the current state of the progress inside the class
    // @WorkerThread
    virtual void store_progress(Progress const&) {}

    // Store the current state of the progress inside the class
    // Use inside the do_in_background()
    // @WorkerThread
    void publish_progress(Progress const& progress)
    {
        // do_in_background() could invoke publish_progress() during dtor(), so publish_progress() must be a non-virtual function and prevent to call virtual ones using is_cancelled()
        if (is_cancelled())
            return;

        this->store_progress(progress);
    }

public:
    // Post process result if it is needed, still on the worker thread
    // @WorkerThread
    virtual Result post_result(Result&& result) { return std::move(result); }

    // Usually to setup the feedback system
    // @MainThread
    virtual void on_pre_execute() {}

    virtual void on_exception(AsyncTaskException* exception) {}

    // Usually to declare the finishing in the feedback system
    // @MainThread
    virtual void on_post_execute(Result const&) {}

    // Cleanup function if the task is canceled
    // @MainThread
    virtual void on_cancelled() {}

    // Cleanup function if the task is canceled
    // @MainThread
    virtual void on_cancelled(Result const&) { on_cancelled(); }

protected:
    virtual void handle_progress() = 0;

public:
    // Returns true if the task is canceled by the cancel()
    // It is usable to break process inside the do_in_background()
    // @MainThread and @Workerthread
    bool is_cancelled() const noexcept { return skr_atomic32_load_relaxed(&cancelled); }

    // Cancel the task
    // @MainThread
    void cancel() noexcept { skr_atomic32_store_relaxed(&cancelled, 1); }

    // Get the result.
    // It could freeze the mainthread if it invoked before the task is finished. Exception from the do_in_background can be rethrown.
    //@MainThread
    Result get()
    {
        if (get_status() != Status::FINISHED)
        {
            finish(future->get());
        }
        return result;
    }

    // Callback loop to refresh progress in the feedback system
    // Return true if the task is finished. Exception from the do_in_background can be rethrown.
    // @MainThread
    bool on_callback_loop()
    {
        if (status == Status::FINISHED)
            return true;

        if (status == Status::PENDING || !future->valid())
            return false;

        auto const future_status = future->wait_for(0);

        switch (future_status)
        {
            case FutureStatus::Deferred:
                return false;

            case FutureStatus::Timeout:
                handle_progress();
                return false;

            case FutureStatus::Ready:
                finish(future->get());
                return true;

            default:
                return false;
        }
    }

private:
    // @MainThread
    void finish(Result&& r)
    {
        result = r;
        if (is_cancelled())
            on_cancelled(result);
        else
            on_post_execute(result);

        status = Status::FINISHED;
    }
};

} // namespace skr