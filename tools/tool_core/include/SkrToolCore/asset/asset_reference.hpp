#pragma once
#include "SkrToolCore/fwd_types.hpp"
#include "utils/types.h"

#ifndef __meta__
#include "SkrToolCore/asset/asset_reference.generated.h"
#endif

namespace skd sreflect
{
namespace asset sreflect
{
sreflect_struct("guid" : "90ca55bc-2be9-4b52-a4e5-cf51974d0932")
sattr("rtti": true, "serialize": ["json", "bin"])
TOOL_CORE_API AssetRef
{
    skr_guid_t guid;
};

sreflect_struct("guid" : "994d203d-72de-4c21-8e1a-a4200ed87ab4")
sattr("rtti": true, "serialize": ["json", "bin"])
TOOL_CORE_API SoftAssetRef
{
    skr_guid_t guid;
};

template<typename AssetT, typename AssetRefT>
struct TAssetRefImpl : public AssetRefT
{

};

template<typename T = void>
struct TAssetRef : public TAssetRefImpl<T, AssetRef> {};

template<typename T = void>
struct TSoftAssetRef : public TAssetRefImpl<T, SoftAssetRef> {};

} // namespace asset
} // namespace skd