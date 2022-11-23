#pragma once
#include "SkrAnimTool/module.configure.h"
#include "platform/configure.h"
#ifndef __meta__
    #include "SkrAnimTool/settings.generated.h"
#endif

namespace ozz {
    enum Endianness : uint32_t;
} // namespace ozz

namespace skd sreflect
{
namespace asset sreflect
{
    sreflect_enum_class("guid" : "2AE2A59C-9C3A-4DB3-8B81-FE22BFC57B0B")
    sattr("serialize" : "json")
    SEndianness : uint32_t
    {
        Native,
        Little,
        Big,
    };

    sreflect_struct("guid" : "7B3FF444-D8E4-42A9-BE36-6E929C168B03")
    sattr("serialize" : "json")
    SKR_ANIMTOOL_API SAnimCookCommonSetting
    {
        SEndianness endianness;
    };

    ozz::Endianness GetEndianness(const SAnimCookCommonSetting& _setting);
}
}