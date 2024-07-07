#pragma once

// This header helps including Ruby across platforms and Ruby versions.

///// Visual Studio CRT Config /////////////////////////////////////////////////

// Disable all warnings.
// http://stackoverflow.com/a/2541990/486990
#pragma warning(push, 0)

// Must disable the min/max macros defined by windows.h to avoid conflict with
// std::max and std:min. Ruby includes windows.h so it must be disabled here.
// http://stackoverflow.com/a/2789509/486990
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

// Ruby 2.5 also define math macros - if they are undefined. Including cmath
// here ensures the stdlib macros are used and avoid redefinition.
#define _USE_MATH_DEFINES
#include <cmath>

// Ruby was configured for Visual Studio 2010. These macros are needed to
// prevent compiler errors with Visual Studio 2013. This relate to release
// builds which are compiled with MT.

#if _MT

// Visual Studio 2013
#if _MSC_VER >= 1800

// Ruby 2.0
#define HAVE_ACOSH 1
#define HAVE_CBRT 1
#define HAVE_ERF 1
#define HAVE_ROUND 1
#define HAVE_TGAMMA 1

#define HAVE_ISINF 1

// Ruby 2.2
#define HAVE_NEXTAFTER 1

#endif // VS 2013

// Visual Studio 2015
#if _MSC_VER >= 1900

#define HAVE_STRUCT_TIMESPEC 1

#endif // VS2015

#endif // _MT


///// Ruby Headers /////////////////////////////////////////////////////////////

// For some reason Xcode will flag warnings (which are treated as errors) in
// the Ruby headers. This works around that.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wdeprecated-register"

#include "ruby.h"
#include "ruby/encoding.h"
// #include <ruby.h>
// #ifdef HAVE_RUBY_ENCODING_H
// #include <ruby/encoding.h>
// #endif

#pragma clang diagnostic pop
#pragma warning (pop)

///// Compatibility Macros /////////////////////////////////////////////////////

/* The structures changes between Ruby 1.8 and 2.0 so the access to the
 * properties are different. There are new macros in Ruby 2.0 that should be
 * used instead. In order to make the code compile for both we need to add
 * matching macros for 1.8.
 */

// #undef write
// #undef read
// #undef bind
