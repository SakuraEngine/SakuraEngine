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

#include "import2ozz_utils.h"

#include <cstdlib>
#include <cstring>
#include <iomanip>

#include "SkrAnimTool/ozz/raw_track.h"
#include "SkrAnimTool/ozz/tools/import2ozz.h"
#include "SkrAnimTool/ozz/track_builder.h"
#include "SkrAnimTool/ozz/track_optimizer.h"
#include "SkrAnim/ozz/skeleton.h"
#include "SkrAnim/ozz/track.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/log.h"
#include "SkrAnim/ozz/base/maths/simd_math.h"
#include "SkrAnim/ozz/base/maths/soa_transform.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"


#include "SkrAnimTool/ozz/raw_skeleton.h"
#include "SkrAnimTool/ozz/skeleton_builder.h"
#include "SkrAnim/ozz/base/containers/map.h"
#include "SkrAnim/ozz/base/containers/set.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/containers/set.h"

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


// Uses a set to detect names uniqueness.
typedef ozz::set<const char*, ozz::str_less> Names;

bool ValidateJointNamesUniquenessRecurse(
    const RawSkeleton::Joint::Children& _joints, Names* _names) {
  for (size_t i = 0; i < _joints.size(); ++i) {
    const RawSkeleton::Joint& joint = _joints[i];
    const char* name = joint.name.c_str();
    if (!_names->insert(name).second) {
      ozz::log::Err()
          << "Skeleton contains at least one non-unique joint name \"" << name
          << "\", which is not supported." << std::endl;
      return false;
    }
    if (!ValidateJointNamesUniquenessRecurse(_joints[i].children, _names)) {
      return false;
    }
  }
  return true;
}

bool ValidateJointNamesUniqueness(const RawSkeleton& _skeleton) {
  Names joint_names;
  return ValidateJointNamesUniquenessRecurse(_skeleton.roots, &joint_names);
}

void LogHierarchy(const RawSkeleton::Joint::Children& _children,
                  int _depth = 0) {
  const std::streamsize pres = ozz::log::LogV().stream().precision();
  for (size_t i = 0; i < _children.size(); ++i) {
    const RawSkeleton::Joint& joint = _children[i];
    ozz::log::LogV() << std::setw(_depth) << std::setfill('.') << "";
    ozz::log::LogV() << joint.name.c_str() << std::setprecision(4)
                     << " t: " << joint.transform.translation.x << ", "
                     << joint.transform.translation.y << ", "
                     << joint.transform.translation.z
                     << " r: " << joint.transform.rotation.x << ", "
                     << joint.transform.rotation.y << ", "
                     << joint.transform.rotation.z << ", "
                     << joint.transform.rotation.w
                     << " s: " << joint.transform.scale.x << ", "
                     << joint.transform.scale.y << ", "
                     << joint.transform.scale.z << std::endl;

    // Recurse
    LogHierarchy(joint.children, _depth + 1);
  }
  ozz::log::LogV() << std::setprecision(static_cast<int>(pres));
}



void DisplaysOptimizationstatistics(const RawAnimation& _non_optimized,
                                    const RawAnimation& _optimized) {
  size_t opt_translations = 0, opt_rotations = 0, opt_scales = 0;
  for (size_t i = 0; i < _optimized.tracks.size(); ++i) {
    const RawAnimation::JointTrack& track = _optimized.tracks[i];
    opt_translations += track.translations.size();
    opt_rotations += track.rotations.size();
    opt_scales += track.scales.size();
  }
  size_t non_opt_translations = 0, non_opt_rotations = 0, non_opt_scales = 0;
  for (size_t i = 0; i < _non_optimized.tracks.size(); ++i) {
    const RawAnimation::JointTrack& track = _non_optimized.tracks[i];
    non_opt_translations += track.translations.size();
    non_opt_rotations += track.rotations.size();
    non_opt_scales += track.scales.size();
  }

  // Computes optimization ratios.
  float translation_ratio = opt_translations != 0
                                ? 1.f * non_opt_translations / opt_translations
                                : 0.f;
  float rotation_ratio =
      opt_rotations != 0 ? 1.f * non_opt_rotations / opt_rotations : 0.f;
  float scale_ratio = opt_scales != 0 ? 1.f * non_opt_scales / opt_scales : 0.f;

  ozz::log::LogV log;
  ozz::log::FloatPrecision precision_scope(log, 1);
  log << "Optimization stage results:" << std::endl;
  log << " - Translations: " << translation_ratio << ":1" << std::endl;
  log << " - Rotations: " << rotation_ratio << ":1" << std::endl;
  log << " - Scales: " << scale_ratio << ":1" << std::endl;
}

vector<math::Transform> SkeletonRestPoseSoAToAoS(const Skeleton& _skeleton) {
  // Copy skeleton rest pose to AoS form.
  vector<math::Transform> transforms(_skeleton.num_joints());
  for (int i = 0; i < _skeleton.num_soa_joints(); ++i) {
    const math::SoaTransform& soa_transform = _skeleton.joint_rest_poses()[i];
    math::SimdFloat4 translation[4];
    math::SimdFloat4 rotation[4];
    math::SimdFloat4 scale[4];
    math::Transpose3x4(&soa_transform.translation.x, translation);
    math::Transpose4x4(&soa_transform.rotation.x, rotation);
    math::Transpose3x4(&soa_transform.scale.x, scale);
    for (int j = 0; j < 4 && i * 4 + j < _skeleton.num_joints(); ++j) {
      math::Transform& out = transforms[i * 4 + j];
      math::Store3PtrU(translation[j], &out.translation.x);
      math::StorePtrU(rotation[j], &out.rotation.x);
      math::Store3PtrU(scale[j], &out.scale.x);
    }
  }
  return transforms;
}
}  // namespace offline
}  // namespace animation
}  // namespace ozz
