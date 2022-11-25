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

#include "import2ozz_skel.h"

#include <cstdlib>
#include <cstring>
#include <iomanip>

#include "SkrAnimTool/ozz/tools/import2ozz.h"

#include "SkrAnimTool/ozz/raw_skeleton.h"
#include "SkrAnimTool/ozz/skeleton_builder.h"

#include "SkrAnim/ozz/skeleton.h"

#include "SkrAnim/ozz/base/containers/map.h"
#include "SkrAnim/ozz/base/containers/set.h"

#include "SkrAnim/ozz/base/io/archive.h"
#include "SkrAnim/ozz/base/io/stream.h"

#include "SkrAnim/ozz/base/memory/unique_ptr.h"

#include "SkrAnim/ozz/base/log.h"

#include "SkrAnim/ozz/base/containers/set.h"

namespace ozz {
namespace animation {
namespace offline {

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
}  // namespace offline
}  // namespace animation
}  // namespace ozz
