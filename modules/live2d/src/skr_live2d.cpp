#include "SkrRT/misc/log.h"
#include "SkrRT/platform/memory.h"
#include "SkrLive2D/skr_live2d.h"
#include "CubismFramework.hpp"

#include <EASTL/fixed_string.h>

/**	Log out func register to cubism */
static void Cubism_Log(const char* message)
{
    eastl::fixed_string<char, 32> buf;
    buf = message;
    // replace the last '\n' with '\0'
    if (buf[buf.size() - 1] == '\n')
        buf[buf.size() - 1] = '\0';
	SKR_LOG_TRACE(u8"[Live2D] %s", buf.c_str());
}

class CubismFrameWorkAllocator : public Csm::ICubismAllocator
{
    void* Allocate(const Csm::csmSizeType size)
    {
        return sakura_malloc(size);
    }

    void Deallocate(void* memory)
    {
        return sakura_free(memory);
    }

    void* AllocateAligned(const Csm::csmSizeType size, const Csm::csmUint32 alignment)
    {
        return sakura_malloc_aligned(size, alignment);
    }

    void DeallocateAligned(void* alignedMemory)
    {
        return sakura_free_aligned(alignedMemory, 1/* ....... */);
    }
};

CubismFrameWorkAllocator ls_Allocator;

void SkrLive2DModule::on_load(int argc, char8_t** argv)
{
    using namespace Live2D::Cubism::Core;

    if (!_framework_initialized)
    {
        csmVersion version = csmGetVersion();
        SKR_UNREF_PARAM(version);

        Csm::CubismFramework::Option _cubismOption;  ///< Cubism SDK Option
        //setup cubism
        _cubismOption.LogFunction = Cubism_Log;
        _cubismOption.LoggingLevel = Live2D::Cubism::Framework::CubismFramework::Option::LogLevel_Verbose;
        auto result = Csm::CubismFramework::StartUp(&ls_Allocator, &_cubismOption);

        if (result)
        {
            //Initialize cubism
            Csm::CubismFramework::Initialize();

            _framework_initialized = true;
        }
        else
        {
            SKR_LOG_ERROR(u8"[Live2D] Failed to initialize framework");
        }
    }

    SKR_LOG_TRACE(u8"live2d module loaded!");
}

void SkrLive2DModule::on_unload()
{
    Csm::CubismFramework::Dispose();
    Csm::CubismFramework::CleanUp();

    SKR_LOG_TRACE(u8"live2d module unloaded!");
}

IMPLEMENT_DYNAMIC_MODULE(SkrLive2DModule, SkrLive2D);