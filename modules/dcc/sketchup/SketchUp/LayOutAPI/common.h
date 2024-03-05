// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_COMMON_H_
#define LAYOUT_COMMON_H_

#define LO_EXPORT
#if (defined WIN32 || defined WIN64) && !defined LO_NO_EXPORTS
  #undef LO_EXPORT
  #ifdef LAYOUT_API_EXPORTS
    #define LO_EXPORT __declspec(dllexport)
  #else
    #define LO_EXPORT __declspec(dllimport)
  #endif  // LAYOUT_API_EXPORTS
#endif    // WINDOWS

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>

#define LO_RESULT LO_EXPORT SUResult

#endif  // LAYOUT_COMMON_H_
