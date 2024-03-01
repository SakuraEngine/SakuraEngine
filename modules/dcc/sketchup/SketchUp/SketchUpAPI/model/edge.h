// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUEdgeRef.
 */
#ifndef SKETCHUP_MODEL_EDGE_H_
#define SKETCHUP_MODEL_EDGE_H_

#include <SketchUpAPI/color.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct SUEdgeRef
 * @extends SUDrawingElementRef
 * @brief  References an edge.
 */

/**
@brief Converts from an \ref SUEdgeRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] edge The given edge reference.
@related SUEdgeRef
@return
- The converted \ref SUEntityRef if edge is a valid edge
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUEdgeToEntity(SUEdgeRef edge);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUEdgeRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an \ref SUEdgeRef.
@param[in] entity The given entity reference.
@related SUEdgeRef
@return
- The converted \ref SUEdgeRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEdgeRef SUEdgeFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUEdgeRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@param[in] edge The given edge reference.
@related SUEdgeRef
@return
- The converted \ref SUEntityRef if edge is a valid edge
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUEdgeToDrawingElement(SUEdgeRef edge);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUEdgeRef.
       This is essentially a downcast operation so the given element must be
       convertible to an \ref SUEdgeRef.
@param[in] drawing_elem The given element reference.
@related SUEdgeRef
@return
- The converted \ref SUEdgeRef if the downcast operation succeeds
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEdgeRef SUEdgeFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a new edge object.

The edge object must be subsequently deallocated with \ref SUEdgeRelease() unless
the edge object is associated with a parent object.
@param[out] edge  The edge object.
@param[in]  start The start position of the edge object.
@param[in]  end   The end position of the edge object.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if start or end is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edge is NULL
- \ref SU_ERROR_GENERIC if start and end specify the same position.
*/
SU_RESULT SUEdgeCreate(SUEdgeRef* edge, const struct SUPoint3D* start, const struct SUPoint3D* end);

/**
@brief Releases an edge object.

The edge object must have been created with SUEdgeCreate() and not
subsequently associated with a parent object (e.g. SUEntitiesAddEdges()).
@param[in] edge The edge object.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge does not reference a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if edge is NULL
*/
SU_RESULT SUEdgeRelease(SUEdgeRef* edge);

/**
@brief Retrieves the associated curve object of an edge object.
@param[in]  edge  The edge object.
@param[out] curve The curve object retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if curve is NULL
- \ref SU_ERROR_NO_DATA if the edge object is not associated with a curve
  object.
*/
SU_RESULT SUEdgeGetCurve(SUEdgeRef edge, SUCurveRef* curve);

/**
@brief Retrieves the starting vertex of an edge object.
@param[in]  edge   The edge object.
@param[out] vertex The vertex object retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid edge object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertex is NULL.
*/
SU_RESULT SUEdgeGetStartVertex(SUEdgeRef edge, SUVertexRef* vertex);

/**
@brief Retrieves the end vertex of an edge object.
@param[in]  edge   The edge object.
@param[out] vertex The vertex object retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertex is NULL
*/
SU_RESULT SUEdgeGetEndVertex(SUEdgeRef edge, SUVertexRef* vertex);

/**
@brief Sets the soft flag of an edge object.
@param[in] edge      The edge object.
@param[in] soft_flag The soft flag to set.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
*/
SU_RESULT SUEdgeSetSoft(SUEdgeRef edge, bool soft_flag);

/**
@brief Retrieves the soft flag of an edge object.
@param[in]  edge      The edge object.
@param[out] soft_flag The soft flag retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if soft_flag is NULL
*/
SU_RESULT SUEdgeGetSoft(SUEdgeRef edge, bool* soft_flag);

/**
@brief Sets the smooth flag of an edge object.
@param[in] edge        The edge object.
@param[in] smooth_flag The smooth flag to set.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
*/
SU_RESULT SUEdgeSetSmooth(SUEdgeRef edge, bool smooth_flag);

/**
@brief Retrieves the smooth flag of an edge object.
@param[in]  edge        The edge object.
@param[out] smooth_flag The smooth flag retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if smooth_flag is NULL
*/
SU_RESULT SUEdgeGetSmooth(SUEdgeRef edge, bool* smooth_flag);

/**
@brief Retrieves the number of faces that the edge is associated with.
@param[in]  edge  The edge object.
@param[out] count The number of faces.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEdgeGetNumFaces(SUEdgeRef edge, size_t* count);

/**
@brief Retrieves the face objects associated with an edge object.
@param[in]  edge  The edge object.
@param[in]  len   The number of faces to retrieve.
@param[out] faces The faces retrieved.
@param[out] count The number of face objects retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if faces or count is NULL
*/
SU_RESULT SUEdgeGetFaces(SUEdgeRef edge, size_t len, SUFaceRef faces[], size_t* count);

/**
@brief Retrieves the color of an edge object.
@param[in]  edge  The edge object.
@param[out] color The color retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUEdgeGetColor(SUEdgeRef edge, SUColor* color);

/**
@brief Computes the length of the edge with the provided transformation applied.
@param[in]  edge      The edge object.
@param[in]  transform A transformation to be appllied to the edge.
@param[out] length    The length retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
SU_RESULT SUEdgeGetLengthWithTransform(
    SUEdgeRef edge, const struct SUTransformation* transform, double* length);

/**
@brief Sets the color of an edge object.
@param[in] edge  The edge object.
@param[in] color The color object to set.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is an invalid object.
- \ref SU_ERROR_NULL_POINTER_INPUT if color is NULL.
*/
SU_RESULT SUEdgeSetColor(SUEdgeRef edge, const SUColor* color);

/**
@brief Determine if \p edge is reversed in \p face 's bounding loop.
@since SketchUp 2021.1, API 9.1
@param [in]  edge The edge object.
@param [in]  face The face object.
@param [out] reversed The reverse status retrieved.
@related SUEdgeRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p edge is an invalid object.
- \ref SU_ERROR_INVALID_INPUT if \p face is an invalid object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p reversed is null.
- \ref SU_ERROR_INVALID_ARGUMENT if \p edge is not bounding \p face.
*/
SU_RESULT SUEdgeReversedInFace(SUEdgeRef edge, SUFaceRef face, bool* reversed);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_EDGE_H_
