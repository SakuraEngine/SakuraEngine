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

#include "SkrAnimTool/ozz/tools/import2ozz.h"

#include "tools/jsoncpp/dist/json/json.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "import2ozz_anim.h"
#include "import2ozz_config.h"
#include "import2ozz_skel.h"
#include "SkrAnim/ozz/base/io/stream.h"
#include "SkrAnim/ozz/base/log.h"

// Declares command line options.


namespace ozz {
namespace animation {
namespace offline {
ozz::string OzzImporter::BuildFilename(const char* _filename,
                                       const char* _data_name) const {
  // Fixup invalid characters for a path.
  ozz::string data_name(_data_name);
  for (const char c : {'<', '>', ':', '/', '\\', '|', '?', '*'}) {
    std::replace(data_name.begin(), data_name.end(), c, '_');
  }

  // Replaces asterisk with data_name
  bool used = false;
  ozz::string output(_filename);
  for (size_t asterisk = output.find('*'); asterisk != std::string::npos;
       used = true, asterisk = output.find('*')) {
    output.replace(asterisk, 1, data_name);
  }

  // Displays a log only if data name was renamed and used as a filename.
  if (used && data_name != _data_name) {
    ozz::log::Log() << "Resource name \"" << _data_name
                    << "\" was changed to \"" << data_name
                    << "\" in order to be used as a valid filename."
                    << std::endl;
  }
  return output;
}
}  // namespace offline
}  // namespace animation
}  // namespace ozz
