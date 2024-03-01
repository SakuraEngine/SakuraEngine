// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SULoopRef.
 */
#ifndef SKETCHUP_MODEL_LOOP_H_
#define SKETCHUP_MODEL_LOOP_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULoopRef
@extends SUEntityRef
@brief  References a loop object, which can be either the outer loop or an inner
        loop (hole) of a face.
*/

/**
@enum SULoopWinding
@brief Indicates loop orientation.
*/
enum SULoopWinding { SULoopWinding_CCW, SULoopWinding_CW };

/**
@brief Converts from an \ref SULoopRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] loop The given loop reference.
@related SULoopRef
@return
- The converted \ref SUEntityRef if loop is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SULoopToEntity(SULoopRef loop);

/**
@brief Converts from an \ref SUEntityRef to an \ref SULoopRef.
       This is essentially a downcast operation so the given SUEntityRef must be
       convertible to an \ref SULoopRef.
@param[in] entity The given entity reference.
@related SULoopRef
@return
- The converted \ref SULoopRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SULoopRef SULoopFromEntity(SUEntityRef entity);

/**
@brief Retrieves the number of vertices of a face loop.
@param[in]  loop  The loop object.
@param[out] count The number of vertices.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SULoopGetNumVertices(SULoopRef loop, size_t* count);

/**
@brief Retrieves the vertices of a face loop object.
@param[in]  loop     The loop object.
@param[in]  len      The number of vertices to retrieve.
@param[out] vertices The vertices retrieved.
@param[out] count    The number of vertices retrieved.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertices or count is NULL
*/
SU_RESULT SULoopGetVertices(SULoopRef loop, size_t len, SUVertexRef vertices[], size_t* count);

/**
@brief Retrieves the edges of a loop object.
@param[in]  loop  The loop object.
@param[in]  len   The number of edges to retrieve.
@param[out] edges The edges retrieved.
@param[out] count The number of edges retrieved.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SULoopGetEdges(SULoopRef loop, size_t len, SUEdgeRef edges[], size_t* count);

/**
@brief Retrieves the  winding of a loop object with respect to a vector.
@param[in]  loop        The loop object.
@param[in]  vector3d    The 3D vector.
@param[out] orientation The orientation retrieved.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if vector3d is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if orientation is NULL
*/
SU_RESULT SULoopGetWinding(
    SULoopRef loop, const struct SUVector3D* vector3d, enum SULoopWinding* orientation);

/**
@brief Retrieves a flag indicating the orientation of the given edge relative to
       a loop object.
@param[in]  loop     The loop object.
@param[in]  edge     The edge object.
@param[out] reversed The flag retrieved. A return value of true indicates that
                     the given edge is oriented opposite of the loop object.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop or edge is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if reversed is NULL
- \ref SU_ERROR_GENERIC if edge is not a part of loop
*/
SU_RESULT SULoopIsEdgeReversed(SULoopRef loop, SUEdgeRef edge, bool* reversed);

/**
@brief Retrieves the parent face of a loop object.
@param[in] loop  The loop object.
@param[out] face The face retrieved.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if face is NULL
- \ref SU_ERROR_OVERWRITE_VALID if face references a valid face object
*/
SU_RESULT SULoopGetFace(SULoopRef loop, SUFaceRef* face);

/**
@brief Retrieves a flag indicating the whether the loop is convex.
@param[in]  loop   The loop object.
@param[out] convex The flag retrieved. A return value of true indicates the
                   loop is convex.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if convex is NULL
*/
SU_RESULT SULoopIsConvex(SULoopRef loop, bool* convex);

/**
@brief Retrieves a flag indicating the whether the loop is the outer loop on its
       associated face.
@param[in]  loop       The loop object.
@param[out] outer_loop The flag retrieved. A return value of true indicates the
                       loop is the outer loop.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if outer_loop is NULL
*/
SU_RESULT SULoopIsOuterLoop(SULoopRef loop, bool* outer_loop);

/**
@brief Retrieves the edge use objects of a loop.
@param[in]  loop      The loop object.
@param[in]  len       The number of edge uses to retrieve.
@param[out] edge_uses The edge uses retrieved.
@param[out] count     The number of edge uses retrieved.
@related SULoopRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edge_uses or count is NULL
*/
SU_RESULT SULoopGetEdgeUses(SULoopRef loop, size_t len, SUEdgeUseRef edge_uses[], size_t* count);

#ifdef __cplusplus
}  // extern "C" {
#endif

#endif  // SKETCHUP_MODEL_LOOP_H_
