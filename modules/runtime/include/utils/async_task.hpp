#pragma once
#include "platform/configure.h"
#include <type_traits>
#include "platform/atomic.h"
#include "platform/thread.h"
#include "platform/memory.h"

#include <EASTL/atomic.h>
#include <EASTL/queue.h>

namespace skr
{

struct AsyncTaskException {
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

template <typename Result>
struct IFuture 
{
    virtual ~IFuture() SKR_NOEXCEPT = default;
    virtual bool valid() const SKR_NOEXCEPT = 0;
    virtual void wait() SKR_NOEXCEPT = 0;
    virtual FutureStatus wait_for(uint32_t ms) SKR_NOEXCEPT = 0;
    virtual Result get() SKR_NOEXCEPT = 0;
};

// AsyncTask
// Asynchronous task progress handler class
//  - Asynchronous worker task should be defined into the do_in_background(),
//  - Feedback system elements should be handled by the on_pre_execute()/on_progress_update()/on_post_execute()/on_cancelled()
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
    virtual ~AsyncTaskBase() SKR_NOEXCEPT
    {
        auto const status = get_status();
        if (status != Status::RUNNING)
            return;

        if (!this->is_cancelled())
            this->cancel();

        if (!this->future->valid())
            return;

        this->future->wait();
        // TODO: recycler
        SkrDelete(future);
    };

public:
    // Initiate the asynchronous task
    // If the task is already began, AsyncTaskIllegalStateException will be thrown
    // @MainThread
    // AsyncTaskBase<Launcher, Progress, Result, Params...>&
    auto& execute(Params const&... params) SKR_NOEXCEPT
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
            SkrDelete(exception);
        }

        this->status = Status::RUNNING;
        this->on_pre_execute();
        this->future = Launcher::async(
            [this](Params const&... params) -> Result 
            {
                if (is_cancelled())
                {
                    return {}; // Protect against undefined behavoiur, if Dtor is invoked before the - pure virtual function represented - task would be started
                }
                return this->post_result(this->do_in_background(params...));
            },
        params...);

        return *this;
    }

    // @MainThread
    Status get_status() const SKR_NOEXCEPT { return status; }

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
    bool is_cancelled() const SKR_NOEXCEPT { return skr_atomic32_load_relaxed(&cancelled); }

    // Cancel the task
    // @MainThread
    void cancel() SKR_NOEXCEPT { skr_atomic32_store_relaxed(&cancelled, 1); }

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
    virtual bool on_callback_loop()
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
        {
            on_cancelled(result);
        }
        else
        {
            on_post_execute(result);
        }
        status = Status::FINISHED;
    }
};

// General AsyncTask
template <typename Launcher, typename Progress, typename Result, typename... Params>
class AsyncTask : public AsyncTaskBase<Launcher, Progress, Result, Params...>
{
private:
    // Progress handling
    template <typename Data>
    struct ThreadSafeContainer {
    private:
        Data mData{};
        mutable SRWMutex mMutex{};

    public:
        ThreadSafeContainer()
        {
            skr_init_rw_mutex(&mMutex);
        }
        ~ThreadSafeContainer()
        {
            skr_destroy_rw_mutex(&mMutex);
        }
        ThreadSafeContainer(ThreadSafeContainer const&) = delete;
        ThreadSafeContainer(ThreadSafeContainer&&) = delete;
        ThreadSafeContainer& operator=(ThreadSafeContainer const&) = delete;
        ThreadSafeContainer& operator=(ThreadSafeContainer&&) = delete;

        void store(Data const& data)
        {
            skr_rw_mutex_acuire_w(&mMutex);
            mData = data;
            skr_rw_mutex_release(&mMutex);
        }

        Data load() const
        {
            skr_rw_mutex_acuire_r(&mMutex);
            const auto data = mData;
            skr_rw_mutex_release(&mMutex);
            return data;
        }
    };

    static bool constexpr isProgressAtomicCompatible =
    std::is_trivially_copyable_v<Progress> && std::is_copy_constructible_v<Progress> && std::is_move_constructible_v<Progress> && std::is_copy_assignable_v<Progress> && std::is_move_assignable_v<Progress>;

    using ProgressContainer = typename std::conditional<
    isProgressAtomicCompatible, eastl::atomic<Progress>, ThreadSafeContainer<Progress>>::type;

    ProgressContainer mProgress;

protected:
    // Store the current state of the progress inside the class
    // @WorkerThread
    void store_progress(Progress const& progress) override
    {
        this->mProgress.store(progress);
    }

    // Show progress in the feedback system
    // @MainThread
    virtual void on_progress_update(Progress const&) {}

protected:
    virtual void handle_progress() override final
    {
        this->on_progress_update(mProgress.load());
    }

public:
    using AsyncTaskBase<Launcher, Progress, Result, Params...>::AsyncTaskBase;
};

template <typename Launcher, typename Progress, typename Result, typename... Params>
class AsyncTaskPQ : public AsyncTaskBase<Launcher, Progress, Result, Params...>
{
private:
    // Progress handling
    template <typename Data>
    struct ThreadSafeQueue {
    private:
        eastl::queue<Data> mData{};
        mutable SRWMutex mMutex{};

    public:
        ThreadSafeQueue() = default;
        ThreadSafeQueue(ThreadSafeQueue const&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue const&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

        template <typename BinaryOp>
        void store(Data const& data, BinaryOp fnLastShouldBeOverride)
        {
            skr_rw_mutex_acuire_w(&mMutex);

            if (mData.empty() || !fnLastShouldBeOverride(mData.back(), data))
            {
                mData.push(data);
            }
            else
            {
                mData.back() = data;
            }

            skr_rw_mutex_release(&mMutex);
        }

        eastl::queue<Data> move()
        {
            skr_rw_mutex_acuire_w(&mMutex);
            auto const mDataMoved = std::move(mData);
            skr_rw_mutex_release(&mMutex);
            return mDataMoved;
        }
    };

    ThreadSafeQueue<Progress> mProgressQueue;

protected:
    // To define condition when not all progress item wanted to be stored.
    // @WorkerThread
    virtual bool isLastProgressShouldBeOverride(Progress const& progressOld, Progress const& progressNew) const { return false; }

    // Store the current state of the progress inside the class
    // @WorkerThread
    void store_progress(Progress const& progress) override
    {
        // or use overrideLast() in special cases.
        this->mProgressQueue.store(progress, [this](Progress const& progressOld, Progress const& progressNew) {
            return this->isLastProgressShouldBeOverride(progressOld, progressNew);
        });
    }

    // Show progress in the feedback system
    // @MainThread
    virtual void on_progress_update(Progress const&) {}

protected:
    virtual void handle_progress() override final
    {
        for (auto progressQueue = mProgressQueue.move(); !progressQueue.empty(); progressQueue.pop())
            this->on_progress_update(progressQueue.front());
    }

public:
    using AsyncTaskBase<Launcher, Progress, Result, Params...>::AsyncTaskBase;
};

} // namespace skr
