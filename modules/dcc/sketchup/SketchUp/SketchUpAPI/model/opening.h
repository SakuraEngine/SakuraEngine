// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUOpeningRef.
 */
#ifndef SKETCHUP_MODEL_OPENING_H_
#define SKETCHUP_MODEL_OPENING_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/component_instance.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUOpeningRef
@brief  References an opening object, which is a hole in a face created by an
        attached component instance or group.
@since SketchUp 2014, API 2.0

*/

/**
@brief Retrieves the number of points of an opening.
@since SketchUp 2014, API 2.0
@param[in]  opening The opening object.
@param[out] count   The number of points.
@related SUOpeningRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if opening is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUOpeningGetNumPoints(SUOpeningRef opening, size_t* count);

/**
@brief Retrieves the points of an opening object.
@since SketchUp 2014, API 2.0
@param[in]  opening  The opening object.
@param[in]  len      The number of points to retrieve.
@param[out] points   The points retrieved.
@param[out] count    The number of points retrieved.
@related SUOpeningRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if opening is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if points or count is NULL
*/
SU_RESULT SUOpeningGetPoints(
    SUOpeningRef opening, size_t len, struct SUPoint3D points[], size_t* count);

/**
@brief Release an opening object.
@since SketchUp 2014, API 2.0
@param[in]  opening  The opening object.
@related SUOpeningRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if opening is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if points or count is NULL
*/
SU_RESULT SUOpeningRelease(SUOpeningRef* opening);

#ifdef __cplusplus
}  // extern "C" {
#endif

#endif  // SKETCHUP_MODEL_OPENING_H_
