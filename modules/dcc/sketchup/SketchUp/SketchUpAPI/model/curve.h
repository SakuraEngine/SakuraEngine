// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUCurveRef.
 */
#ifndef SKETCHUP_MODEL_CURVE_H_
#define SKETCHUP_MODEL_CURVE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUCurveRef
@extends SUEntityRef
@brief  References a curve.
*/

/**
@enum SUCurveType
@brief Defines curve types that can be represented by \ref SUCurveRef.
*/
enum SUCurveType { SUCurveType_Simple = 0, SUCurveType_Arc };

/**
@brief Converts from an \ref SUCurveRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] curve The given curve reference.
@related SUCurveRef
@return
- The converted \ref SUEntityRef if curve is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUCurveToEntity(SUCurveRef curve);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUCurveRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUCurveRef.
@param[in] entity The given entity reference.
@related SUCurveRef
@return
- The converted \ref SUCurveRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUCurveRef SUCurveFromEntity(SUEntityRef entity);

/**
@brief  Creates a curve object with the given array of edges that is not
        connected to any face object.  The array of N edges is sorted such that
        for each edge in the range [0, N] the start position of each edge is the
        same as the end position of the previous edge in the array.  Each
        element of the array of edges is subsequently associated with the
        created curve object and must not be deallocated via SUEdgeRelease().
@param curve The curve object created.
@param edges The array of edge objects.
@param len   The number of edge objects in the array.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if edges is NULL
- \ref SU_ERROR_OUT_OF_RANGE if len is 0
- \ref SU_ERROR_NULL_POINTER_OUTPUT if curve is NULL
- \ref SU_ERROR_OVERWRITE_VALID if curve already references a valid object
- \ref SU_ERROR_GENERIC if edge array contains an invalid edge, if the edges
  in the array are not connected, if any of the edges are associated with a face
  object, or the edges describe a loop
*/
SU_RESULT SUCurveCreateWithEdges(SUCurveRef* curve, const SUEdgeRef edges[], size_t len);

/**
@brief  Releases a curve object and its associated edge objects.
@param curve The curve object.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if curve is NULL
- \ref SU_ERROR_INVALID_INPUT if curve does not reference a valid object
*/
SU_RESULT SUCurveRelease(SUCurveRef* curve);

/**
@brief Retrieves the curve type of a curve object.
@param[in]  curve The curve object.
@param[out] type  The curve type retrieved.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if curve is not a valid curve object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUCurveGetType(SUCurveRef curve, enum SUCurveType* type);

/**
@brief Retrieves the number of edges that belong to a curve object.
@param[in]  curve The curve object.
@param[out] count The number of edges.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if curve is not a valid curve object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUCurveGetNumEdges(SUCurveRef curve, size_t* count);

/**
@brief Retrieves the edges of a curve object. Provides access to all edges in
       the curve. The first edge is the first element of the edges array and
       the last edge is the last element if all edges were retrieved.
@param[in]  curve The curve object.
@param[in]  len   The number of edges to retrieve.
@param[out] edges The edges retrieved.
@param[out] count The number of edges retrieved.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if curve is not a valid curve object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SUCurveGetEdges(SUCurveRef curve, size_t len, SUEdgeRef edges[], size_t* count);

/**
@brief True if this edge was originally created by the polygon tool, otherwise false.
@since SketchUp 2021.1, API 9.1
@param[in]  curve
@param[out] is_curve The edges retrieved.
@related SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p curve is not a valid curve object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p is_curve is NULL
*/
SU_RESULT SUCurveIsPolygon(SUCurveRef curve, bool* is_curve);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_CURVE_H_
