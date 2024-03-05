// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUVertexRef.
 */
#ifndef SKETCHUP_MODEL_VERTEX_H_
#define SKETCHUP_MODEL_VERTEX_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUVertexRef
@extends SUEntityRef
@brief  A vertex type that has a position and is associated with other geometry
        like edges, faces, and loops.
*/

/**
@brief  Converts from an \ref SUVertexRef to an \ref SUEntityRef.  This is
        essentially an upcast operation.
@param[in] vertex The vertex object.
@related SUVertexRef
@return
- The converted \ref SUEntityRef if vertex is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUVertexToEntity(SUVertexRef vertex);

/**
@brief  Converts from an \ref SUEntityRef to an \ref SUVertexRef.  This is
        essentially a downcast operation so the given \ref SUEntityRef must be
        convertible to an \ref SUVertexRef.
@param[in] entity The entity object.
@related SUVertexRef
@return
- The converted \ref SUVertexRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUVertexRef SUVertexFromEntity(SUEntityRef entity);

/**
@brief  Retrieves the position of a vertex object.
@param[in]  vertex   The vertex object.
@param[out] position The vertex position.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if position is NULL
*/
SU_RESULT SUVertexGetPosition(SUVertexRef vertex, struct SUPoint3D* position);

/**
@brief  Sets the position of a vertex object.
@param[in] vertex   The vertex object.
@param[in] position The value used to set the vertex position.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if position is NULL
*/
SU_RESULT SUVertexSetPosition(SUVertexRef vertex, const struct SUPoint3D* position);

/**
@brief  Retrieves the number of edges that the vertex is associated with.
@param[in]  vertex The vertex object.
@param[out] count  The number of edges.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUVertexGetNumEdges(SUVertexRef vertex, size_t* count);

/**
@brief  Retrieves the edge objects associated with a vertex object.
@param[in]  vertex The vertex object.
@param[in]  len    The number of edges to retrieve.
@param[out] edges  The edges retrieved.
@param[out] count  The number of edges retrieved.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SUVertexGetEdges(SUVertexRef vertex, size_t len, SUEdgeRef edges[], size_t* count);

/**
@brief  Retrieves the number of faces that the vertex is associated with.
@param[in]  vertex The vertex object.
@param[out] count  The number of faces.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUVertexGetNumFaces(SUVertexRef vertex, size_t* count);

/**
@brief  Retrieves the face objects associated with a vertex object.
@param[in]  vertex The vertex object.
@param[in]  len    The number of faces to retrieve.
@param[out] faces  The faces retrieved.
@param[out] count  The number of faces retrieved.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if faces or count is NULL
*/
SU_RESULT SUVertexGetFaces(SUVertexRef vertex, size_t len, SUFaceRef faces[], size_t* count);

/**
@brief  Retrieves the number of loops that the vertex is associated with.
@param[in]  vertex The vertex object.
@param[out] count  The number of loops.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUVertexGetNumLoops(SUVertexRef vertex, size_t* count);

/**
@brief  Retrieves the loop objects associated with a vertex object.
@param[in]  vertex The vertex object.
@param[in]  len    The number of loops to retrieve.
@param[out] loops  The loops retrieved.
@param[out] count  The number of loops retrieved.
@related SUVertexRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if vertex is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if loops or count is NULL
*/
SU_RESULT SUVertexGetLoops(SUVertexRef vertex, size_t len, SULoopRef loops[], size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_VERTEX_H_
