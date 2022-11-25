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

#ifndef OZZ_ANIMATION_OFFLINE_TOOLS_IMPORT2OZZ_SKEL_H_
#define OZZ_ANIMATION_OFFLINE_TOOLS_IMPORT2OZZ_SKEL_H_

#include "SkrAnim/ozz/base/endianness.h"
#include "SkrAnim/ozz/base/platform.h"
#include "SkrAnimTool/ozz/tools/import2ozz.h"

namespace Json {
class Value;
}

namespace ozz {
namespace animation {
namespace offline {

class OzzImporter;
struct RawSkeleton;

OZZ_ANIMTOOLS_DLL void DisplaysOptimizationstatistics(const RawAnimation& _non_optimized,
                                    const RawAnimation& _optimized);

OZZ_ANIMTOOLS_DLL vector<math::Transform> SkeletonRestPoseSoAToAoS(const Skeleton& _skeleton);

// Additive reference enum to config string conversions.
struct AdditiveReferenceEnum {
  enum Value { kAnimation, kSkeleton };
};
OZZ_ANIMTOOLS_DLL bool ValidateJointNamesUniqueness(const RawSkeleton& _skeleton);

}  // namespace offline
}  // namespace animation
}  // namespace ozz
#endif  // OZZ_ANIMATION_OFFLINE_TOOLS_IMPORT2OZZ_SKEL_H_
