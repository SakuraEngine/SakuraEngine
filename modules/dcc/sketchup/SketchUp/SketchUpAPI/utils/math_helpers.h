// Copyright 2015 Trimble Inc. All rights reserved.

/**
 * @file
 * @brief Interfaces for comparing values with tolerances.
 */
#ifndef SKETCHUP_UTILS_MATH_HELPERS_H_
#define SKETCHUP_UTILS_MATH_HELPERS_H_

#include <SketchUpAPI/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Converts a value from degrees to radians.
@since SketchUp 2018, API 6.0
@param[in] value A value in degrees.
@return    The value converted to radians.
*/
SU_EXPORT double SUDegreesToRadians(double value);

/**
@brief Converts a value from radians to degrees.
@since SketchUp 2018, API 6.0
@param[in] value A value in radians.
@return    The value converted to degrees.
*/
SU_EXPORT double SURadiansToDegrees(double value);

/**
@brief Compares two values for equality with a tolerance.
@since SketchUp 2018, API 6.0
@param[in] val1 The first value.
@param[in] val2 The second value.
@return    True if the values are equal.
*/
SU_EXPORT bool SUEquals(double val1, double val2);

/**
@brief Compares two values with a tolerance to see if val1 is less than val2.
@since SketchUp 2018, API 6.0
@param[in] val1 The first value.
@param[in] val2 The second value.
@return    True if val1 is less than val2.
*/
SU_EXPORT bool SULessThan(double val1, double val2);

/**
@brief Compares two values with a tolerance to see if val1 is less than or equal
       to val2.
@since SketchUp 2018, API 6.0
@param[in] val1 The first value.
@param[in] val2 The second value.
@return    True if val1 is less than or equal to val2.
*/
SU_EXPORT bool SULessThanOrEqual(double val1, double val2);

/**
@brief Compares two values with a tolerance to see if val1 is greater than val2.
@since SketchUp 2018, API 6.0
@param[in] val1 The first value.
@param[in] val2 The second value.
@return    True if val1 is greater than val2.
*/
SU_EXPORT bool SUGreaterThan(double val1, double val2);

/**
@brief Compares two values with a tolerance to see if val1 is greater than or
       equal to val2.
@since SketchUp 2018, API 6.0
@param[in] val1 The first value.
@param[in] val2 The second value.
@return    True if val1 is greater than or equal to val2.
*/
SU_EXPORT bool SUGreaterThanOrEqual(double val1, double val2);

// SketchUp 2017 added these functions, but lacked the SU* prefix. As of
// SketchUp 2018 they were renamed. This compatibility macro is left around
// until SketchUp 2019. Enable to temporarily re-enable the old function names.
#ifdef SU_COMPAT_MATH_UTILS

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static double DegreesToRadians(double value) {
  return SUDegreesToRadians(value);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static bool Equals(double val1, double val2) {
  return SUEquals(val1, val2);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static bool GreaterThan(double val1, double val2) {
  return SUGreaterThan(val1, val2);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static bool GreaterThanOrEqual(double val1, double val2) {
  return SUGreaterThanOrEqual(val1, val2);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static bool LessThan(double val1, double val2) {
  return SULessThan(val1, val2);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static bool LessThanOrEqual(double val1, double val2) {
  return SULessThanOrEqual(val1, val2);
}

SU_DEPRECATED_FUNCTION("SketchUp API 6.0")
static double RadiansToDegrees(double value) {
  return SURadiansToDegrees(value);
}
#endif

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // SKETCHUP_UTILS_MATH_HELPERS_H_
