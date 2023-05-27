#include "platform/windows/windows_dstorage.hpp"

#include "EASTL/vector_map.h"
#include "EASTL/algorithm.h"

SkrWindowsDStorageInstance* SkrWindowsDStorageInstance::Get()
{
    static SkrWindowsDStorageInstance* _this = nullptr;
    if (!_this)
    {
        _this = SkrNew<SkrWindowsDStorageInstance>();
        {
            _this->dstorage_core.load(u8"dstoragecore.dll");
            _this->dstorage_library.load(u8"dstorage.dll");
            if (!_this->dstorage_core.isLoaded() || !_this->dstorage_library.isLoaded())
            {
                if (!_this->dstorage_core.isLoaded()) SKR_LOG_TRACE("dstoragecore.dll not found, direct storage is disabled");
                if (!_this->dstorage_library.isLoaded()) SKR_LOG_TRACE("dstorage.dll not found, direct storage is disabled");
                _this->dstorage_dll_dont_exist = true;
            }
            else
            {
                SKR_LOG_TRACE("dstorage.dll loaded");

                auto pfn_get_factory = SKR_SHARED_LIB_LOAD_API(_this->dstorage_library, DStorageGetFactory);
                if (!pfn_get_factory) return nullptr;
                
                if (!SUCCEEDED(pfn_get_factory(IID_PPV_ARGS(&_this->pFactory))))
                {
                    SKR_LOG_ERROR("Failed to get DStorage factory!");
                    return nullptr;
                }
            }
        }
    }
    return _this->dstorage_dll_dont_exist ? nullptr : _this;
}


SkrWindowsDStorageInstance::~SkrWindowsDStorageInstance()
{
    if (pFactory) pFactory->Release();
    if (dstorage_core.isLoaded()) dstorage_core.unload();
    if (dstorage_library.isLoaded()) dstorage_library.unload();
    
    SKR_LOG_TRACE("Direct Storage unloaded");
}

bool created = false;
SkrDStorageInstanceId skr_create_dstorage_instance(SkrDStorageConfig* config)
{
    created = true;
    return SkrWindowsDStorageInstance::Get();
}

SkrDStorageInstanceId skr_get_dstorage_instnace()
{
    SKR_ASSERT(created && 
        "Direct Storage instance not created, "
        "you must call 'skr_create_dstorage_instance' at first!");
    return SkrWindowsDStorageInstance::Get();
}

ESkrDStorageAvailability skr_query_dstorage_availability()
{
    auto inst = SkrWindowsDStorageInstance::Get();
    return inst ? SKR_DSTORAGE_AVAILABILITY_HARDWARE : SKR_DSTORAGE_AVAILABILITY_NONE;
}

void skr_free_dstorage_instance(SkrDStorageInstanceId inst)
{
    SkrDelete(inst);
}

SkrDStorageQueueId skr_create_dstorage_queue(const SkrDStorageQueueDescriptor* desc)
{
    auto _this = (SkrWindowsDStorageInstance*)skr_get_dstorage_instnace();
    if (!_this) return nullptr;

    DStorageQueueWindows* Q = SkrNew<DStorageQueueWindows>();
    DSTORAGE_QUEUE_DESC queueDesc{};
    queueDesc.Capacity = desc->capacity;
    queueDesc.Priority = (DSTORAGE_PRIORITY)desc->priority;
    Q->source_type = queueDesc.SourceType = (DSTORAGE_REQUEST_SOURCE_TYPE)desc->source;
    queueDesc.Name = (const char*)desc->name;
    queueDesc.Device = nullptr;
    IDStorageFactory* pFactory = _this->pFactory;
    if (!pFactory) return nullptr;
    if (!SUCCEEDED(pFactory->CreateQueue(&queueDesc, IID_PPV_ARGS(&Q->pQueue))))
    {
        SKR_LOG_ERROR("Failed to create DStorage queue!");
        SkrDelete(Q);
        return nullptr;
    }
#ifdef TRACY_PROFILE_DIRECT_STORAGE
    skr_init_mutex_recursive(&Q->profile_mutex);
#endif
    Q->max_size = _this->sDirectStorageStagingBufferSize;
    Q->pFactory = pFactory;
    Q->device = nullptr;
    return Q;
}

void skr_free_dstorage_queue(SkrDStorageQueueId queue)
{
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;

    if (Q->pQueue)
    {
        // TODO: WaitErrorEvent & Handle errors 
        DSTORAGE_ERROR_RECORD record = {};
        Q->pQueue->RetrieveErrorRecord(&record);

#ifdef TRACY_PROFILE_DIRECT_STORAGE
        skr_destroy_mutex(&Q->profile_mutex);
#endif

        Q->pQueue->Release();
    }
    SkrDelete(Q);
}

#include <filesystem>

SkrDStorageFileHandle skr_dstorage_open_file(SkrDStorageQueueId queue, const char* abs_path)
{
    IDStorageFile* pFile = nullptr;
    DStorageQueueWindows* Q = (DStorageQueueWindows*)queue;
    auto absPath = std::filesystem::path(abs_path);
    Q->pFactory->OpenFile(absPath.c_str(), IID_PPV_ARGS(&pFile));
    return (SkrDStorageFileHandle)pFile;
}

void skr_dstorage_query_file_info(SkrDStorageQueueId queue, SkrDStorageFileHandle file, SkrDStorageFileInfo* info)
{
    BY_HANDLE_FILE_INFORMATION fileInfo;
    IDStorageFile* pFile = (IDStorageFile*)file;
    if (!SUCCEEDED(pFile->GetFileInformation(&fileInfo)))
    {
        SKR_LOG_ERROR("Failed to get DStorage file info!");
        return;
    }
    info->file_size = fileInfo.nFileSizeLow;
    if (fileInfo.nFileSizeHigh)
    {
        info->file_size += ((uint64_t)fileInfo.nFileSizeHigh << 32);
    }
}

void skr_dstorage_close_file(SkrDStorageQueueId queue, SkrDStorageFileHandle file)
{
    IDStorageFile* pFile = (IDStorageFile*)file;
    pFile->Close();
    pFile->Release();
}
