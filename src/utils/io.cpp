#include "utils/io.h"
#include "utils/io.hpp"
#include <ftl/task_counter.h>
#include "platform/memory.h"

bool skr_async_io_request_t::is_ready() const
{
    return SKR_ASYNC_IO_STATUS_OK == skr_atomic32_load_acquire(&status);
}

eastl::unique_ptr<ftl::TaskScheduler> skr::io::RAM::ramTaskScheduler;
void skr::io::RAM::initialize()
{
    auto ftl_options = ftl::TaskSchedulerInitOptions();
    ftl_options.ThreadPoolSize = 2;
    ramTaskScheduler = eastl::make_unique<ftl::TaskScheduler>();
    ramTaskScheduler->Init(ftl_options);
}

struct VFSReadTaskData {
    skr_async_io_request_t* request;
    ftl::Task task;
    ftl::TaskCounter taskCounter;
    uint32_t file_count;
    uint8_t** bytes;
    skr_vfile_t** files;
    uint64_t* offsets;
    uint64_t* sizes;
};

void vfsReadTask(ftl::TaskScheduler* taskScheduler, void* arg)
{
    auto data = (VFSReadTaskData*)arg;
    skr_atomic32_store_release(&data->request->status, SKR_ASYNC_IO_STATUS_RAM_LOADING);
    for (uint32_t i = 0; i < data->file_count; i++)
    {
        skr_vfs_fread(data->files[i], data->bytes[i], data->offsets[i], data->sizes[i]);
        skr_vfs_fclose(data->files[i]);
    }
    skr_atomic32_store_release(&data->request->status, SKR_ASYNC_IO_STATUS_OK);
    // clean up
    sakura_free(data);
}

void skr::io::RAM::request(skr_vfs_t* vfs, const char8_t* path,
uint8_t* bytes, uint64_t offset, uint64_t size,
skr_async_io_request_t* request)
{
    if (false /*support async vfs api*/)
    {
    }
    else if (vfs->procs.fread != nullptr)
    {
#define FILE_COUNT 1
        VFSReadTaskData* data = (VFSReadTaskData*)sakura_malloc(
        sizeof(VFSReadTaskData) +
        (sizeof(uint8_t) + sizeof(skr_vfile_t*) + sizeof(uint64_t) + sizeof(uint64_t)) * 1);
        ftl::Task* task = new (&data->task) ftl::Task;
        ftl::TaskCounter* taskCounter = new (&data->taskCounter) ftl::TaskCounter(ramTaskScheduler.get());
        // begins block data
        data->bytes = (uint8_t**)(data + 1);
        data->files = (skr_vfile_t**)(data->bytes + FILE_COUNT);
        data->offsets = (uint64_t*)(data->files + FILE_COUNT);
        data->sizes = (uint64_t*)(data->offsets + FILE_COUNT);
        data->file_count = FILE_COUNT;
        // record request data
        request->counter = (struct skr_async_io_request_counter_t*)taskCounter;
        request->task = (struct skr_async_io_task_t*)task;
        request->status = SKR_ASYNC_IO_STATUS_NONE;
        *task = { vfsReadTask, data };
        for (uint32_t i = 0; i < FILE_COUNT; i++)
        {
            data->files[i] = skr_vfs_fopen(vfs, path,
            ESkrFileMode::SKR_FM_READ,
            ESkrFileCreation::SKR_FILE_CREATION_OPEN_EXISTING);
            data->offsets[i] = offset;
            data->sizes[i] = size;
            data->bytes[i] = bytes;
        }
        data->request = request;
        ramTaskScheduler->AddTasks(1, task, ftl::TaskPriority::Normal, taskCounter);
    }
}

// C API
void skr_io_ram_initialize()
{
    return skr::io::RAM::initialize();
}

void skr_io_ram_request(skr_vfs_t* vfs,
const char8_t* path, uint8_t* bytes,
uint64_t offset, uint64_t size, skr_async_io_request_t* request)
{
    return skr::io::RAM::request(vfs, path, bytes, offset, size, request);
}