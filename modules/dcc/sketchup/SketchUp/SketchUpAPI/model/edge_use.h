// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUEdgeUseRef.
 */
#ifndef SKETCHUP_MODEL_EDGE_USE_H_
#define SKETCHUP_MODEL_EDGE_USE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

/**
@struct SUEdgeUseRef
@extends SUEntityRef
@brief SUEdgeUseRef objects are used to retrieve the topology of the edges of
       a polygon. The geometry of the polygon being represented by \ref
       SULoopRef that is already associated with a face object. The typical use
       of EdgeUse object is to retrieve them from a face object's loop, and then
       read the topology values from them.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Converts from an \ref SUEdgeUseRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] edgeuse The given edge use reference.
@related SUEdgeUseRef
@return
- The converted \ref SUEntityRef if edgeuse is a valid edge use.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEntityRef SUEdgeUseToEntity(SUEdgeUseRef edgeuse);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUEdgeUseRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUEdgeUseRef.
@param[in] entity The given entity reference.
@related SUEdgeUseRef
@return
- The converted \ref SUEdgeUseRef if the downcast operation succeeds.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEdgeUseRef SUEdgeUseFromEntity(SUEntityRef entity);

/**
@brief Retrieves the edge object the EdgeUse object belongs to.
@param[in]  edgeuse The EdgeUse object.
@param[out] edge    The edge object retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edge is NULL.
*/
SU_RESULT SUEdgeUseGetEdge(SUEdgeUseRef edgeuse, SUEdgeRef* edge);

/**
@brief Retrieves the loop object the EdgeUse object is associated with.
@param[in]  edgeuse The EdgeUse object.
@param[out] loop    The loop object retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if loop is NULL
*/
SU_RESULT SUEdgeUseGetLoop(SUEdgeUseRef edgeuse, SULoopRef* loop);

/**
@brief Retrieves the face object the EdgeUse object is associated with.
@param[in]  edgeuse The EdgeUse object.
@param[out] face    The face object retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if face is NULL
*/
SU_RESULT SUEdgeUseGetFace(SUEdgeUseRef edgeuse, SUFaceRef* face);

/**
@brief Retrieves the number of EdgeUse objects that are linked to the EdgeUse
       object.
@param[in]  edgeuse The EdgeUse object.
@param[out] count   The number of partners.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEdgeUseGetNumPartners(SUEdgeUseRef edgeuse, size_t* count);

/**
@brief Retrieves the EdgeUse objects that are linked to the EdgeUse object.
@param[in]  edgeuse  The EdgeUse object.
@param[in]  len      The number of partners to retrieve.
@param[out] partners The partners retrieved.
@param[out] count    The number of partners retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if partners or count is NULL
*/
SU_RESULT SUEdgeUseGetPartners(
    SUEdgeUseRef edgeuse, size_t len, SUEdgeUseRef partners[], size_t* count);

/**
@brief Retrieves a flag indicating whether this EdgeUse is traversed in the
       opposite direction as its corresponding edge.
@param[in]  edgeuse The EdgeUse object.
@param[out] reversed The retrieved flag.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if reversed is NULL.
*/
SU_RESULT SUEdgeUseIsReversed(SUEdgeUseRef edgeuse, bool* reversed);

/**
@brief Retrieves the EdgeUse object just preceding an EdgeUse object in the
       collection of linked EdgeUses.
@param[in]  edgeuse      The EdgeUse object.
@param[out] prev_edgeuse The EdgeUse retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if prev_edgeuse is NULL.
*/
SU_RESULT SUEdgeUseGetPrevious(SUEdgeUseRef edgeuse, SUEdgeUseRef* prev_edgeuse);

/**
@brief Retrieves the EdgeUse object just following an EdgeUse object in the
       collection of linked EdgeUses.
@param[in]  edgeuse      The EdgeUse object.
@param[out] next_edgeuse The EdgeUse retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edgeuse is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if next_edgeuse is NULL.
*/
SU_RESULT SUEdgeUseGetNext(SUEdgeUseRef edgeuse, SUEdgeUseRef* next_edgeuse);

/**
@brief Retrieves the start vertex of an EdgeUse object. The start vertex of the
       EdgeUse object may not be the same as the start vertex of the
       corresponding edge of the EdgeUse object. An EdgeUse object is part of a
       face loop whose direction may be the reverse of the direction of the
       edge.
@param[in]  edgeuse The EdgeUse object.
@param[out] vertex  The vertex object retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid EdgeUse object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertex is NULL
*/
SU_RESULT SUEdgeUseGetStartVertex(SUEdgeUseRef edgeuse, SUVertexRef* vertex);

/**
@brief Retrieves the end vertex of an EdgeUse object.

The end vertex of the EdgeUse object may not be the same as the end vertex of
the corresponding edge of the EdgeUse object. An EdgeUse object is part of a
face loop whose direction may be the reverse of the direction of the edge.
@param[in]  edgeuse The EdgeUse object.
@param[out] vertex  The vertex object retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid EdgeUse object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertex is NULL
*/
SU_RESULT SUEdgeUseGetEndVertex(SUEdgeUseRef edgeuse, SUVertexRef* vertex);

/**
@brief Retrieves the normal vector at the start vertex of an EdgeUse object.
@param[in]  edgeuse The EdgeUse object.
@param[out] normal  The normal vector retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL
*/
SU_RESULT SUEdgeUseGetStartVertexNormal(SUEdgeUseRef edgeuse, struct SUVector3D* normal);

/**
@brief Retrieves the normal vector at the end vertex of an EdgeUse object.
@param[in]  edgeuse The EdgeUse object.
@param[out] normal  The normal vector retrieved.
@related SUEdgeUseRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if edge is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL.
*/
SU_RESULT SUEdgeUseGetEndVertexNormal(SUEdgeUseRef edgeuse, struct SUVector3D* normal);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_EDGE_USE_H_
