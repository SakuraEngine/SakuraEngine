// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUMeshHelperRef.
 */
#ifndef SKETCHUP_MODEL_MESH_HELPER_H_
#define SKETCHUP_MODEL_MESH_HELPER_H_

#include <stdlib.h>

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/texture_writer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUMeshHelperRef
@brief  A helper class that will tessellate a \ref SUFaceRef object into
        triangles, and then provide the vertices, normals, and STQ coordinates
        of those triangles.
*/

/**
@brief  Creates a tessellated polygon mesh object from a face object.
@param[in] mesh_ref  The mesh object created.
@param[in] face_ref  The face object.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if mesh_ref is NULL
- \ref SU_ERROR_OVERWRITE_VALID if mesh_ref already references a valid object
*/
SU_RESULT SUMeshHelperCreate(SUMeshHelperRef* mesh_ref, SUFaceRef face_ref);

/**
@brief  Creates a tessellated polygon mesh object from a face object and the
        texture writer object used to write the material texture(s) of the face
        object.
@param[out] mesh_ref           The mesh object created.
@param[in]  face_ref           The face object.
@param[in]  texture_writer_ref The texture writer object.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face_ref or texture_writer_ref is not a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if mesh_ref is NULL
- \ref SU_ERROR_OVERWRITE_VALID if mesh_ref already references a valid object
*/
SU_RESULT SUMeshHelperCreateWithTextureWriter(
    SUMeshHelperRef* mesh_ref, SUFaceRef face_ref, SUTextureWriterRef texture_writer_ref);

/**
@brief  Creates a tessellated polygon mesh object from a face and a UV helper
        associated with the face.
@param[out] mesh_ref      The mesh object created.
@param[in]  face_ref      The face object.
@param[in]  uv_helper_ref The UV helper object.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face_ref or uv_helper_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if mesh_ref is NULL
- \ref SU_ERROR_OVERWRITE_VALID if mesh_ref already references a valid object
*/
SU_RESULT SUMeshHelperCreateWithUVHelper(
    SUMeshHelperRef* mesh_ref, SUFaceRef face_ref, SUUVHelperRef uv_helper_ref);

/**
@brief  Deallocates a polygon mesh object.
@param[in] mesh_ref The mesh object to deallocate.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if mesh_ref is NULL
*/
SU_RESULT SUMeshHelperRelease(SUMeshHelperRef* mesh_ref);

/**
@brief  Retrieves the total number of polygons in the mesh.
@param[in]  mesh_ref The mesh object.
@param[out] count    The number of polygons available.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if mesh_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUMeshHelperGetNumTriangles(SUMeshHelperRef mesh_ref, size_t* count);

/**
@brief  Retrieves the total number of vertices in the polygon mesh object.
@param[in]  mesh_ref The mesh object.
@param[out] count    The number of vertices available.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if mesh_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUMeshHelperGetNumVertices(SUMeshHelperRef mesh_ref, size_t* count);

/**
@brief  Retrieves the array of indices of the vertices of a triangle mesh
        object. The each element indexes into the arrays retrieved with \ref
        SUMeshHelperGetVertices, \ref SUMeshHelperGetFrontSTQCoords(), \ref
        SUMeshHelperGetBackSTQCoords and \ref SUMeshHelperGetNormals().  The
        elements are sorted so that every three elements (i.e., stride of three)
        compose the indices to the three vertices of a triangle.
@param[in]  mesh_ref The mesh object.
@param[in]  len      The number of indices to retrieve.
@param[out] indices  The indices retrieved.
@param[out] count    The number of indices retrieved.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if indices or count is NULL
*/
SU_RESULT SUMeshHelperGetVertexIndices(
    SUMeshHelperRef mesh_ref, size_t len, size_t indices[], size_t* count);

/**
@brief  Retrieves the vertices of a triangle mesh object.
@param[in]  mesh_ref The mesh object.
@param[in]  len      The number of vertices to retrieve.
@param[out] vertices The vertices retrieved.
@param[out] count    The number of vertices retrieved.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertices or count is NULL
*/
SU_RESULT SUMeshHelperGetVertices(
    SUMeshHelperRef mesh_ref, size_t len, struct SUPoint3D vertices[], size_t* count);

/**
@brief  Retrieves the front stq texture coordinates of a triangle mesh object.
@param[in]  mesh_ref The mesh object.
@param[in]  len      The number of stq coordinates to retrieve.
@param[out] stq      The stq coordinates retrieved.
@param[out] count    The number of stq coordinates retrieved.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if stq or count is NULL
*/
SU_RESULT SUMeshHelperGetFrontSTQCoords(
    SUMeshHelperRef mesh_ref, size_t len, struct SUPoint3D stq[], size_t* count);

/**
@brief  Retrieves the back stq texture coordinates of a triangle mesh object.
@param[in]  mesh_ref The mesh object.
@param[in]  len      The number of stq coordinates to retrieve.
@param[out] stq      The stq coordinates retrieved.
@param[out] count    The number of stq coordinates retrieved.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if stq or count is NULL
*/
SU_RESULT SUMeshHelperGetBackSTQCoords(
    SUMeshHelperRef mesh_ref, size_t len, struct SUPoint3D stq[], size_t* count);

/**
@brief  Retrieves the vertex normal vectors of a triangle mesh object.
@param[in]  mesh_ref The mesh object whose vertex normal vectors are retrieved.
@param[in]  len      The number of vertex normal objects to retrieve.
@param[out] normals  The vertex normal vectors retrieved.
@param[out] count    The number of normal vectors retrieved.
@related SUMeshHelperRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normals or count is NULL
*/
SU_RESULT SUMeshHelperGetNormals(
    SUMeshHelperRef mesh_ref, size_t len, struct SUVector3D normals[], size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_MESH_HELPER_H_
