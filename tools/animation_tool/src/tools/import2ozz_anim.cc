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

#include "import2ozz_anim.h"

#include <cstdlib>
#include <cstring>

#include "import2ozz_track.h"
#include "SkrAnimTool/ozz/additive_animation_builder.h"
#include "SkrAnimTool/ozz/animation_builder.h"
#include "SkrAnimTool/ozz/animation_optimizer.h"
#include "SkrAnimTool/ozz/raw_animation.h"
#include "SkrAnimTool/ozz/raw_skeleton.h"
#include "SkrAnimTool/ozz/skeleton_builder.h"
#include "SkrAnimTool/ozz/tools/import2ozz.h"
#include "SkrAnim/ozz/animation.h"
#include "SkrAnim/ozz/skeleton.h"
#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/log.h"
#include "SkrAnim/ozz/base/maths/soa_transform.h"
#include "SkrAnim/ozz/base/memory/unique_ptr.h"

namespace ozz {
namespace animation {
namespace offline {

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

unique_ptr<ozz::animation::Skeleton> LoadSkeleton(const char* _path) {
  // Reads the skeleton from the binary ozz stream.
  unique_ptr<ozz::animation::Skeleton> skeleton;
  {
    if (*_path == 0) {
      ozz::log::Err() << "Missing input skeleton file from json config."
                      << std::endl;
      return nullptr;
    }
    ozz::log::LogV() << "Opens input skeleton ozz binary file: " << _path
                     << std::endl;
    ozz::io::File file(_path, "rb");
    if (!file.opened()) {
      ozz::log::Err() << "Failed to open input skeleton ozz binary file: \""
                      << _path << "\"" << std::endl;
      return nullptr;
    }
    ozz::io::IArchive archive(&file);

    // File could contain a RawSkeleton or a Skeleton.
    if (archive.TestTag<RawSkeleton>()) {
      ozz::log::LogV() << "Reading RawSkeleton from file." << std::endl;

      // Reading the skeleton cannot file.
      RawSkeleton raw_skeleton;
      archive >> raw_skeleton;

      // Builds runtime skeleton.
      ozz::log::LogV() << "Builds runtime skeleton." << std::endl;
      SkeletonBuilder builder;
      skeleton = builder(raw_skeleton);
      if (!skeleton) {
        ozz::log::Err() << "Failed to build runtime skeleton." << std::endl;
        return nullptr;
      }
    } else if (archive.TestTag<Skeleton>()) {
      // Reads input archive to the runtime skeleton.
      // This operation cannot fail.
      skeleton = make_unique<Skeleton>();
      archive >> *skeleton;
    } else {
      ozz::log::Err() << "Failed to read input skeleton from binary file: "
                      << _path << std::endl;
      return nullptr;
    }
  }
  return skeleton;
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
