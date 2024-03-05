// Copyright 2013-2021 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUFaceRef.
 */
#ifndef SKETCHUP_MODEL_FACE_H_
#define SKETCHUP_MODEL_FACE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/geometry_input.h>
#include <SketchUpAPI/model/defs.h>

// Forward declarations

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUFaceRef
@extends SUDrawingElementRef
@brief  References a face.
*/

/**
@brief Converts from an \ref SUFaceRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] face The given face reference.
@related SUFaceRef
@return
- The converted SUEntityRef if face is a valid face.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUEntityRef SUFaceToEntity(SUFaceRef face);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUFaceRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an \ref SUFaceRef.
@param[in] entity The given entity reference.
@related SUFaceRef
@return
- The converted SUFaceRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUFaceRef SUFaceFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUFaceRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@param[in] face The given face reference.
@related SUFaceRef
@return
- The converted \ref SUEntityRef if face is a valid face.
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUDrawingElementRef SUFaceToDrawingElement(SUFaceRef face);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUFaceRef.
       This is essentially a downcast operation so the given element must be
       convertible to an SUFaceRef.
@param[in] drawing_elem The given element reference.
@related SUFaceRef
@return
- The converted \ref SUFaceRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUFaceRef SUFaceFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a face without holes.

@bug SUEntitiesAddFaces() will not merge overlapping vertices and edges, which
  produces SketchUp models with unexpected state. Avoid using these functions
  and instead use SUGeometryInputRef along with SUEntitiesFill().

@param[out] face       The face object created.
@param[in]  vertices3d The array of vertices that make the face.
@param[in]  outer_loop The loop input that describes the outer loop of the face.
                       If the function is successful, the new face will take
                       ownership of the loop and this reference will be
                       invalidated.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vertices3d or outer_loop is NULL
- \ref SU_ERROR_INVALID_INPUT if outer_loop contains invalid input data
- \ref SU_ERROR_GENERIC if the the points specified by outer_loop do not lie on
  a plane within 1.0e-3 tolerance
- \ref SU_ERROR_NULL_POINTER_OUTPUT if face is NULL
- \ref SU_ERROR_OVERWRITE_VALID if face already refers to a valid object
*/
SU_RESULT SUFaceCreate(
    SUFaceRef* face, const struct SUPoint3D vertices3d[], SULoopInputRef* outer_loop);

/**
@brief Creates a simple face without holes from an array of vertices.

@bug SUEntitiesAddFaces() will not merge overlapping vertices and edges, which
  produces SketchUp models with unexpected state. Avoid using these functions
  and instead use SUGeometryInputRef along with SUEntitiesFill().

@param[out] face       The face object created.
@param[in]  vertices3d The array of vertices of the face.
@param[in]  len        The length of the array of vertices.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vertices3d is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if face is NULL
- \ref SU_ERROR_GENERIC if the specified vertices do not lie on a plane within
  1.0e-3 tolerance
- \ref SU_ERROR_OVERWRITE_VALID if face already refers to a valid face object
*/
SU_RESULT SUFaceCreateSimple(SUFaceRef* face, const struct SUPoint3D vertices3d[], size_t len);

/**
@brief Retrieves the normal vector of a face object.
@param[in]  face   The face object.
@param[out] normal The 3d normal vector retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if normal is NULL
*/
SU_RESULT SUFaceGetNormal(SUFaceRef face, struct SUVector3D* normal);

/**
@brief Releases a face object and its associated resources.
@param[in] face The face object.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if face points to an NULL
- \ref SU_ERROR_INVALID_INPUT if the face object is not a valid object.
*/
SU_RESULT SUFaceRelease(SUFaceRef* face);

/**
@brief Retrieves the number of edges in a face.
@param[in]  face  The face object.
@param[out] count The number of edges.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumEdges(SUFaceRef face, size_t* count);

/**
@brief Retrieves the edges in the face.
@param[in]  face  The face object.
@param[in]  len   The number of edges to retrieve.
@param[out] edges The edges retrieved.
@param[out] count The number of edges retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SUFaceGetEdges(SUFaceRef face, size_t len, SUEdgeRef edges[], size_t* count);

/**
@brief Retrieves the number of edge uses in a face.
@since SketchUp 2016, API 4.0
@param[in]  face  The face object.
@param[out] count The number of edge uses.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumEdgeUses(SUFaceRef face, size_t* count);

/**
@brief Retrieves the edge uses in the face.
@since SketchUp 2016, API 4.0
@param[in]  face  The face object.
@param[in]  len   The number of edge uses to retrieve.
@param[out] edges The edgeuses retrieved.
@param[out] count The number of edge uses retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SUFaceGetEdgeUses(SUFaceRef face, size_t len, SUEdgeUseRef edges[], size_t* count);

/**
@brief Retrieves the plane of the face.
@param[in]  face  The face object.
@param[out] plane The 3d plane retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUFaceGetPlane(SUFaceRef face, struct SUPlane3D* plane);

/**
@brief Retrieves the number of vertices that the face is associated with.
@param[in]  face  The face object.
@param[out] count The number of vertices.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumVertices(SUFaceRef face, size_t* count);

/**
@brief Retrieves the vertex objects associated with a face object.
@param[in]  face     The face object.
@param[in]  len      The number of vertices to retrieve.
@param[out] vertices The vertices retrieved.
@param[out] count    The number of vertices retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertices or count is NULL
*/
SU_RESULT SUFaceGetVertices(SUFaceRef face, size_t len, SUVertexRef vertices[], size_t* count);

/**
@brief Retrieves the outer loop of a face object.
@param[in]  face The face object.
@param[out] loop The loop object retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if loop is NULL
- \ref SU_ERROR_OVERWRITE_VALID if loop already refers to a valid object
*/
SU_RESULT SUFaceGetOuterLoop(SUFaceRef face, SULoopRef* loop);

/**
@brief Retrieves the number of loops in a face.
@param[in]  face  The face object.
@param[out] count The number of inner loops.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumInnerLoops(SUFaceRef face, size_t* count);

/**
@brief Retrieves the loops in the face.
@param[in]  face  The face object.
@param[in]  len   The number of loops to retrieve.
@param[out] loops The inner loops retrieved.
@param[out] count The number of inner loops retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face object is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if loops or count is NULL
*/
SU_RESULT SUFaceGetInnerLoops(SUFaceRef face, size_t len, SULoopRef loops[], size_t* count);

/**
@brief Adds a hole to the face. The face object must be associated with a parent
       component.
@param[in] face       The face object.
@param[in] vertices3d The array of vertices references by the added loop.
@param[in] loop       The loop input that describes the inner loop.
                      If the function is successful, the new face will take
                      ownership of the loop and this reference will be
                      invalidated.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if vertices3d or loop is NULL
- \ref SU_ERROR_INVALID_INPUT if loop contains invalid input data
- \ref SU_ERROR_GENERIC if the face object is not associated with a parent
  component.
*/
SU_RESULT SUFaceAddInnerLoop(
    SUFaceRef face, const struct SUPoint3D vertices3d[], SULoopInputRef* loop);

/**
@brief Retrieves the number of openings in a face.
@since SketchUp 2014, API 2.0
@param[in]  face  The face object.
@param[out] count The number of openings.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumOpenings(SUFaceRef face, size_t* count);

/**
@brief Retrieves the openings in the face. The retrieved SUOpeningRef() objects
       must be manually released by calling SUOpeningRelease() on each one.
@since SketchUp 2014, API 2.0
@param[in]  face     The face object.
@param[in]  len      The number of openings to retrieve.
@param[out] openings The openings retrieved.
@param[out] count    The number of openings retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if openings or count is NULL
*/
SU_RESULT SUFaceGetOpenings(SUFaceRef face, size_t len, SUOpeningRef openings[], size_t* count);
/**
@brief Retrieves the front material associated with a face object.
@param[in]  face     The face object.
@param[out] material The material object retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if material is NULL
*/
SU_RESULT SUFaceGetFrontMaterial(SUFaceRef face, SUMaterialRef* material);

/**
@brief Sets the front material of a face object.
@param[in] face     The face object.
@param[in] material The material object to set. If invalid, this will set the
                    material to the default material.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUFaceSetFrontMaterial(SUFaceRef face, SUMaterialRef material);

/**
@brief Retrieves the back material associate with a face object.
@param[in]  face     The face object.
@param[out] material The material object retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if material is NULL
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUFaceGetBackMaterial(SUFaceRef face, SUMaterialRef* material);

/**
@brief Sets the back material of a face object.
@param[in] face     The face object.
@param[in] material The material object to set. If invalid, this will set the
                    material to the default material.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
*/
SU_RESULT SUFaceSetBackMaterial(SUFaceRef face, SUMaterialRef material);

/**
@brief Retrieves the flag indicating whether a face object has an affine
       texture applied to its front.
@param[in]  face      The face object.
@param[out] is_affine The flag retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_affine is NULL
- \ref SU_ERROR_NO_DATA if face does not have a textured material applied to its
  front
*/
SU_RESULT SUFaceIsFrontMaterialAffine(SUFaceRef face, bool* is_affine);

/**
@brief Retrieves the flag indicating whether a face object has an affine
       texture applied to its back.
@param[in]  face      The face object.
@param[out] is_affine The flag retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_affine is NULL
- \ref SU_ERROR_NO_DATA if face does not have a textured material applied to its
  back
*/
SU_RESULT SUFaceIsBackMaterialAffine(SUFaceRef face, bool* is_affine);

/**
@brief Computes the area of the face, taking into account all the inner loops
       and cuts from openings.
@param[in]  face The face object.
@param[out] area The area retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if area is NULL
*/
SU_RESULT SUFaceGetArea(SUFaceRef face, double* area);

/**
@brief Computes the area of the face with the provided transformation applied.
//@since SketchUp 2016, API 4.0
@param[in]  face      The face object.
@param[in]  transform A transformation to be appllied to the face.
@param[out] area      The area retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if transformation is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if area is NULL
*/
SU_RESULT SUFaceGetAreaWithTransform(
    SUFaceRef face, const struct SUTransformation* transform, double* area);

/**
@brief Retrieves a flag indicating whether the face is complex, i.e. contains
       either inner loops or openings cut by attached component instances or
       attached groups.
@param[in]  face       The face object.
@param[out] is_complex The flag retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid face object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_complex is NULL
*/
SU_RESULT SUFaceIsComplex(SUFaceRef face, bool* is_complex);

/**
@brief Creates a UV helper for the face.
@param[in]  face  The face object.
@param[in]  front Flag indicating whether to compute the UV coordinates for
                  the front of the face.
@param[in]  back  Flag indicating whether to compute the UV coordinates for
                  the back of the face.
@param[in]  texture_writer An optional texture writer to aid in texture
                           coordinate calculations for non-affine textures.
@param[out] uv_helper      The UV helper object created. Must be deallocated
                           via SUUVHelperRelease().
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uv_helper is NULL
*/
SU_RESULT SUFaceGetUVHelper(
    SUFaceRef face, bool front, bool back, SUTextureWriterRef texture_writer,
    SUUVHelperRef* uv_helper);

/**
@brief Creates a UV helper for the face given a specific texture handle.
@param[in] face  The face object.
@param[in] front Flag indicating whether to compute the UV coordinates for
                 the front of the face.
@param[in] back  Flag indicating whether to compute the UV coordinates for
                 the back of the face.
@param[in] texture_writer An optional texture writer to aid in texture
                          coordinate calculations for non-affine textures.
@param[in] textureHandle  The handle of the image that should be mapped to the
                          face.
@param[out] uv_helper     The UV helper object created.  Must be deallocated
                          via SUUVHelperRelease().
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if uv_helper is NULL.
*/
SU_RESULT SUFaceGetUVHelperWithTextureHandle(
    SUFaceRef face, bool front, bool back, SUTextureWriterRef texture_writer, long textureHandle,
    SUUVHelperRef* uv_helper);

/**
@brief Retrieves the number of attached drawing elements in a face.
@since SketchUp 2016, API 4.0
@param[in]  face  The face object.
@param[out] count The number of attached drawing elements.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUFaceGetNumAttachedDrawingElements(SUFaceRef face, size_t* count);

/**
@brief Retrieves the attached drawing elements in the face.
@since SketchUp 2016, API 4.0
@param[in]  face  The face object.
@param[in]  len   The number of attached drawing elements to retrieve.
@param[out] elems The attached drawing elements retrieved.
@param[out] count The number of attached drawing elements retrieved.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if elems or count is NULL
*/
SU_RESULT SUFaceGetAttachedDrawingElements(
    SUFaceRef face, size_t len, SUDrawingElementRef elems[], size_t* count);

/**
@brief Reverses a face object.
@since SketchUp 2016, API 4.0
@param[in] face The face object.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if face is not a valid object
*/
SU_RESULT SUFaceReverse(SUFaceRef face);

/**
@brief Retrieves the world and UV coordinates of a UV tile.

The UV tile bounds the given reference point on the plane of the face. If the
reference isn't on the plane of the face it will be projected onto it.

The world coordinates are on the plane of the face unless the texture is
projected. When the texture is projected the the world points are on an
arbitrary plane that is perpendicular to the projection direction.

The returned \p points and \p uv_coords can be used directly with
\ref SUMaterialPositionInput.

@since SketchUp 2021.1, API 9.1

@param[in] face
@param[in] reference A point of reference on the plane of the face.
@param[in] front Flag indicating whether to compute the UV Tile for the front
                 side of the face.
@param[out] points World coordinates for the bounding UV tile.
@param[out] uv_coords UV coordinates for the bounding UV tile.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid object.
- \ref SU_ERROR_NO_DATA if the \p face has no textured material of the given side.
- \ref SU_ERROR_NULL_POINTER_INPUT if \p reference is NULL.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p points is NULL.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p uv_coords is NULL.
*/
SU_RESULT SUFaceGetUVTileAt(
    SUFaceRef face, const struct SUPoint3D* reference, bool front, struct SUPoint3D points[4],
    struct SUPoint2D uv_coords[4]);

/**
@brief Retrieves a flag indicating whether the face has a positioned texture.

A texture is positioned when it's not using the default texture coordinates.

When a user uses the Paint Bucket Tool to apply a material sampled from the
Material Browser it will use default texture coordinates and not be
positioned. It will be positioned if the user uses the Position Texture Tool.

@since SketchUp 2021.1, API 9.1

@param[in]  face
@param[in] front  Flag indicating whether to check the front or back side of the face.
@param[out] is_positioned
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p is_positioned is NULL.
*/
SU_RESULT SUFaceIsTexturePositioned(SUFaceRef face, bool front, bool* is_positioned);

/**
@brief Clears the texture position.

This is similar to reseting the texture position from the SketchUp UI face context menu.

@since SketchUp 2022.0, API 10.0

@param[in] face
@param[in] front  Flag indicating whether to clear the front or back side of the face.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object.
*/
SU_RESULT SUFaceClearTexturePosition(SUFaceRef face, bool front);

/**
@brief Retrieves a flag indicating whether the face has a projected texture.

A texture is projected when the user enables this property via the Position Texture Tool.

In SketchUp a projected texture can use an arbitrary plane, rather than the face's own plane. The
projection vector defines the normal of this plane and used this to project the texture onto the
face.

@since SketchUp 2021.1, API 9.1

@param[in]  face
@param[in] front  Flag indicating whether to check the front or back side of the face.
@param[out] is_projected
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p is_projected is NULL.
*/
SU_RESULT SUFaceIsTextureProjected(SUFaceRef face, bool front, bool* is_projected);

/**
@brief Retrieves a vector representing the projection direction for the texture on
       the given side of the face.

@since SketchUp 2021.1, API 9.1

@param[in]  face
@param[in] front  Flag indicating whether to use the front or back side of the face.
@param[out] vector
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object.
- \ref SU_ERROR_NO_DATA if \p face does not have a projected material applied.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p vector is NULL.
*/
SU_RESULT SUFaceGetTextureProjection(SUFaceRef face, bool front, struct SUVector3D* vector);

/**
@brief Clears the texture projection.

This is similar to toggling off Projection from the Position Texture tool in the UI.

@since SketchUp 2021.1, API 9.1

@param[in]  face
@param[in] front  Flag indicating whether to clear the front or back side of the face.
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object.
*/
SU_RESULT SUFaceClearTextureProjection(SUFaceRef face, bool front);

/**
@brief Positions a textured material on the face.

The texture is positioned using 1 to 4 pairs of model and uv points. The points are used in pairs to
describe where a point in the texture image is positioned on the face. This is similar to how
textures are positioned in the UI using the Position Texture tool.

@since SketchUp 2021.1, API 9.1

@param[in] face
@param[in] front  Flag indicating whether to use the front or back side of the face.
@param[in] mapping
@related SUFaceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p face is not a valid face object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p mapping is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if \p mapping doesn't have a valid textured material
- \ref SU_ERROR_INVALID_ARGUMENT if \p mapping doesn't have 1-4 points/uv_coords.
- \ref SU_ERROR_INVALID_ARGUMENT if valid texture coordinates could not be computed from \p mapping.
*/
SU_RESULT SUFacePositionMaterial(
    SUFaceRef face, bool front, struct SUMaterialPositionInput* mapping);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_FACE_H_
