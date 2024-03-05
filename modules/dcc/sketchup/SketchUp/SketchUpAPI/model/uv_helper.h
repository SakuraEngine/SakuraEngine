// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUUVHelperRef.
 */
#ifndef SKETCHUP_MODEL_UV_HELPER_H_
#define SKETCHUP_MODEL_UV_HELPER_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUUVHelperRef
@brief  Used to compute UV texture coordinates for a particular face.
*/

/**
@struct SUUVQ
@brief Stores UV texture coordinates.
*/
struct SUUVQ {
  double u;  ///< U coordinate
  double v;  ///< V coordinate
  double q;  ///< Q coordinate
};

/**
@brief  Releases a UVHelper object that was obtained from a face.
@param[in] uvhelper The UV helper object.
@related SUUVHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if uvhelper is NULL
- \ref SU_ERROR_INVALID_INPUT if uvhelper is an invalid object
*/
SU_RESULT SUUVHelperRelease(SUUVHelperRef* uvhelper);

/**
@brief  Retrieves the UVQ point at the given point for the front of the face.
@param[in]  uvhelper The UV helper object.
@param[in]  point    The point where the coordinates are to be computed.
@param[out] uvq      The coordinates retrieved.
@related SUUVHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if uvhelper is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if point is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uvq is NULL
*/
SU_RESULT SUUVHelperGetFrontUVQ(
    SUUVHelperRef uvhelper, const struct SUPoint3D* point, struct SUUVQ* uvq);

/**
@brief  Retrieves the UVQ point at the given point for the back of the face.
@param[in]  uvhelper The UVHelper object.
@param[in]  point    The point where the coordinates are to be computed.
@param[out] uvq      The coordinates retrieved.
@related SUUVHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if uvhelper is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if point is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uvq is NULL
*/
SU_RESULT SUUVHelperGetBackUVQ(
    SUUVHelperRef uvhelper, const struct SUPoint3D* point, struct SUUVQ* uvq);

#ifdef __cplusplus
}  //  extern "C" {
#endif
#pragma pack(pop)

#endif  // SKETCHUP_MODEL_UV_HELPER_H_
