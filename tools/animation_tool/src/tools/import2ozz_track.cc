//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) Guillaume Blanc                                              //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#include "import2ozz_track.h"

#include <cstdlib>
#include <cstring>

#include "SkrAnimTool/ozz/raw_track.h"
#include "SkrAnimTool/ozz/tools/import2ozz.h"
#include "SkrAnimTool/ozz/track_builder.h"
#include "SkrAnimTool/ozz/track_optimizer.h"
#include "SkrAnim/ozz/skeleton.h"
#include "SkrAnim/ozz/track.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/log.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"

namespace ozz {
namespace animation {
namespace offline {
template <typename _Track>
void DisplaysOptimizationstatistics(const _Track& _non_optimized,
                                    const _Track& _optimized) {
  const size_t opt = _optimized.keyframes.size();
  const size_t non_opt = _non_optimized.keyframes.size();

  // Computes optimization ratios.
  float ratio = opt != 0 ? 1.f * non_opt / opt : 0.f;

  ozz::log::LogV log;
  ozz::log::FloatPrecision precision_scope(log, 1);
  log << "Optimization stage results: " << ratio << ":1" << std::endl;
}

template
void DisplaysOptimizationstatistics(const RawFloatTrack& _non_optimized,
                                    const RawFloatTrack& _optimized);

template
void DisplaysOptimizationstatistics(const RawFloat2Track& _non_optimized,
                                    const RawFloat2Track& _optimized);

template
void DisplaysOptimizationstatistics(const RawFloat3Track& _non_optimized,
                                    const RawFloat3Track& _optimized);

template
void DisplaysOptimizationstatistics(const RawFloat4Track& _non_optimized,
                                    const RawFloat4Track& _optimized);

bool IsCompatiblePropertyType(OzzImporter::NodeProperty::Type _src,
                              OzzImporter::NodeProperty::Type _dest) {
  if (_src == _dest) {
    return true;
  }
  switch (_src) {
    case OzzImporter::NodeProperty::kFloat3:
      return _dest == OzzImporter::NodeProperty::kPoint ||
             _dest == OzzImporter::NodeProperty::kVector;
    case OzzImporter::NodeProperty::kPoint:
    case OzzImporter::NodeProperty::kVector:
      return _dest == OzzImporter::NodeProperty::kFloat3;
    default:
      return false;
  }
}

template <typename _RawTrack>
struct RawTrackToTrack;

template <>
struct RawTrackToTrack<RawFloatTrack> {
  typedef FloatTrack Track;
};
template <>
struct RawTrackToTrack<RawFloat2Track> {
  typedef Float2Track Track;
};
template <>
struct RawTrackToTrack<RawFloat3Track> {
  typedef Float3Track Track;
};
template <>
struct RawTrackToTrack<RawFloat4Track> {
  typedef Float4Track Track;
};
}  // namespace offline
}  // namespace animation
}  // namespace ozz
