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

#include "SkrAnim/ozz/base/memory/allocator.h"

#include <memory.h>

#include <atomic>
#include <cassert>
#include <cstdlib>

#include "SkrAnim/ozz/base/maths/math_ex.h"

#include "platform/memory.h"

namespace ozz {
namespace memory {

void* Allocator::Allocate(size_t _size, size_t _alignment)
{
  return sakura_malloc_aligned(_size, _alignment);
}
void Allocator::Deallocate(void* _block, size_t _alignment)
{
  sakura_free_aligned(_block, _alignment);
}

namespace {
Allocator g_skr_allocator;
}  // namespace

// Implements default allocator accessor.
Allocator* default_allocator() { return &g_skr_allocator; }
}  // namespace memory
}  // namespace ozz
