//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!! THIS FILE IS GENERATED, ANY CHANGES WILL BE LOST !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#pragma once
#include "SkrBase/config.h"
#include <inttypes.h>

#ifdef __meta__
#error "this file should not be inspected by meta"
#endif

#ifdef SKR_FILE_ID
    #undef SKR_FILE_ID
#endif
#define SKR_FILE_ID FID_SkrGui_backend_resource_resource_h_meta

// BEGIN forward declarations
namespace skr::gui { struct IResource; }
namespace skr::gui { struct ISurface; }
namespace skr::gui { struct IMaterial; }
namespace skr::gui { struct IImage; }
namespace skr::gui { struct IUpdatableImage; }


namespace skr::gui { enum class EPixelFormat : int; }

namespace skr::gui { enum class EResourceState : uint32_t; }
// END forward declarations
// BEGIN RTTR GENERATED
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"
namespace skr::rttr
{
// enum traits
}

// rttr traits
SKR_RTTR_TYPE(::skr::gui::IResource, "1d4ba81f-09b7-4186-b35a-f380c49302e4")
SKR_RTTR_TYPE(::skr::gui::ISurface, "f0a63d5a-62ae-44fa-8f8f-6847af623cea")
SKR_RTTR_TYPE(::skr::gui::IMaterial, "41d08a49-c9fe-4ccb-a91d-fd16f946aca1")
SKR_RTTR_TYPE(::skr::gui::IImage, "6cc1395f-9660-4431-b998-df32d1d363eb")
SKR_RTTR_TYPE(::skr::gui::IUpdatableImage, "7ae28a98-10f2-44c4-b7aa-b50780435d03")
// END RTTR GENERATED