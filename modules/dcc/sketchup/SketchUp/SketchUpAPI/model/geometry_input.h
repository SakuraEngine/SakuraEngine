// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUGeometryInputRef.
 */
#ifndef SKETCHUP_MODEL_GEOMETRY_INPUT_H_
#define SKETCHUP_MODEL_GEOMETRY_INPUT_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/curve.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUGeometryInputRef
@brief  References a geometry input object. It is used as an input to
        SUEntitiesFill().
*/

/**
@brief SUMaterialInput contains information that is needed to apply a
       material to a face.

@deprecated This struct is made obsolete by the newer SUMaterialPositionInput
            and left in place only for compatibility.

@note Prior to SketchUp 2021.1, API 9.1 it was not possible to apply a
      textured material without providing explicit UV coordinates.

@see SUMaterialPositionInput

The conventional method for applying a material to a face is to use 1 to 4 UV
coordinates, which are Cartesian textures coordinates and corresponding vertex
indices on the face. The vertices are referenced by index into the top level
SUGeometryInputRef's vertex array. Once the \p material input is used
(e.g. with SUEntitiesFill()), the material object must not be released since it
will be associated with a parent object.
*/
struct SUMaterialInput {
  size_t num_uv_coords;  ///< Number of texture coordinates. 0 if the material
                         ///< should be applied using default UV coordinates.
                         ///< 1 to 4 otherwise.

  struct SUPoint2D uv_coords[4];  ///< Texture coordinates.

  size_t vertex_indices[4];  ///< Vertex indices corresponding to the texture
                             ///< coordinates. Should reference the vertex array
                             ///< of the parent SUGeometryInputRef.

  SUMaterialRef material;  ///< Material to be applied.
};

/**
@brief SUMaterialInput contains information that is needed to apply a
       material to a face.

The conventional method for applying a material to a face is to use 1 to 4 UV
coordinates, which are Cartesian textures coordinates and corresponding 3D
points on the face's plane. If the 3D points are not on the face's plane they
will be projected onto it. Once the material input is used
(e.g. with SUEntitiesFill()), the \p material object must not be released since
it will be associated with a parent object.

@since SketchUp 2021.1, API 9.1
*/
struct SUMaterialPositionInput {
  size_t num_uv_coords;  ///< Number of texture coordinates. 0 if the material
                         ///< should be applied using default UV coordinates.
                         ///< 1 to 4 otherwise.

  struct SUPoint2D uv_coords[4];  ///< Texture coordinates.

  struct SUPoint3D points[4];  ///< 3D point corresponding to the
                               ///< texture coordinates. The points
                               ///< should all lie on the plane of the
                               ///< face the material is applied to.

  SUMaterialRef material;        ///< Material to be applied.
  struct SUVector3D projection;  ///< Optional projection direction.
                                 ///< Leave as invalid (0, 0, 0) for no projection.
};

/**
@brief Creates a geometry input object.
@param[out] geom_input The object created. This object can be passed into
                       SUEntitiesFill() to populate an entities object.
                       It should be released subsequently by calling
                       SUGeometryInputRelease().
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if geom_input is NULL
- \ref SU_ERROR_OVERWRITE_VALID if geom_input already references a valid object
*/
SU_RESULT SUGeometryInputCreate(SUGeometryInputRef* geom_input);

/**
@brief Deallocates a geometry input object.
@param[in] geom_input The object to deallocate.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if geom_input is NULL
- \ref SU_ERROR_INVALID_INPUT if geom_input points to an invalid object
*/
SU_RESULT SUGeometryInputRelease(SUGeometryInputRef* geom_input);

/**
@brief Adds a vertex to a geometry input object.
@param[in] geom_input The geometry input object.
@param[in] point      The location of the vertex to be added.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if point is NULL
*/
SU_RESULT SUGeometryInputAddVertex(SUGeometryInputRef geom_input, const struct SUPoint3D* point);

/**
@brief Sets all vertices of a geometry input object. Any existing vertices will
       be overridden.
@param[in] geom_input   The geometry input object.
@param[in] num_vertices The number of vertices in the given point array.
@param[in] points       The points array containing the location of vertices.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if points is NULL
*/
SU_RESULT SUGeometryInputSetVertices(
    SUGeometryInputRef geom_input, size_t num_vertices, const struct SUPoint3D points[]);

/**
@brief Adds an edge to a geometry input object. This method is intended for
       specifying edges which are not associated with loop inputs. For
       specifying edge properties on a face use the SULoopInput interface.
@since SketchUp 2017, API 5.0
@param[in]  geom_input       The geometry input object.
@param[in]  vertex0_index    The vertex index of the edge's first vertex.
@param[in]  vertex1_index    The vertex index of the edge's last vertex.
@param[out] added_edge_index (optional) If not NULL, returns the index of the
                             added edge.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if vertex0_index or vertex1_index are greater than
       the total vertex count
- \ref SU_ERROR_INVALID_ARGUMENT if vertex0_index and vertex1_index are equal
*/
SU_RESULT SUGeometryInputAddEdge(
    SUGeometryInputRef geom_input, size_t vertex0_index, size_t vertex1_index,
    size_t* added_edge_index);

/**
@brief Sets the hidden flag of an edge in a geometry input object which is not
       associated with a loop input.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] edge_index The zero-based index of the edge which is not associated
                      with a loop input.
@param[in] hidden     The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current total
       number of edges which are not associated with loop inputs
*/
SU_RESULT SUGeometryInputEdgeSetHidden(
    SUGeometryInputRef geom_input, size_t edge_index, bool hidden);

/**
@brief Sets the soft flag of an edge in a geometry input object which is not
       associated with a loop input.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] edge_index The zero-based index of the edge which is not associated
                      with a loop input.
@param[in] soft       The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current total
       number of edges which are not associated with loop inputs
*/
SU_RESULT SUGeometryInputEdgeSetSoft(SUGeometryInputRef geom_input, size_t edge_index, bool soft);

/**
@brief Sets the smooth flag of an edge in a geometry input object which is not
       associated with a loop input.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] edge_index The zero-based index of the edge which is not associated
                      with a loop input.
@param[in] smooth     The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current total
       number of edges which are not associated with loop inputs
*/
SU_RESULT SUGeometryInputEdgeSetSmooth(
    SUGeometryInputRef geom_input, size_t edge_index, bool smooth);

/**
@brief Sets the material of an edge in the geometry input.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] edge_index Index of the edge to set the material.
@param[in] material   The material to be set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input or material is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references an edge beyond the total
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
edge count of geom_input
*/
SU_RESULT SUGeometryInputEdgeSetMaterial(
    SUGeometryInputRef geom_input, size_t edge_index, SUMaterialRef material);

/**
@brief Sets the layer of an edge in the geometry input.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] edge_index Index of the edge to set the layer.
@param[in] layer      The layer to be set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input or layer is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references an edge beyond the total
edge count of geom_input
*/
SU_RESULT SUGeometryInputEdgeSetLayer(
    SUGeometryInputRef geom_input, size_t edge_index, SULayerRef layer);

/**
@brief Adds a curve to a geometry input object. This method is intended for
       specifying curves which are not associated with loop inputs. For
       specifying curves on faces use the SULoopInput interface.
@since SketchUp 2017, API 5.0
@param[in]  geom_input        The geometry input object.
@param[in]  num_edges         The number of edges to be used in the curve.
@param[in]  edge_indices      The edge indices to be used in defining the curve.
@param[out] added_curve_index (optional) If not NULL, returns the index of the
                               added curve.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if edge_indices is NULL
- \ref SU_ERROR_OUT_OF_RANGE if any of the indices in edge_indices are greater
       than the total number of edges which are not associated with loop inputs
*/
SU_RESULT SUGeometryInputAddCurve(
    SUGeometryInputRef geom_input, size_t num_edges, const size_t edge_indices[],
    size_t* added_curve_index);

/**
@brief Adds an arccurve to a geometry input object. In addition to adding an
       arccurve to the geometry input this method will append num_segments edges
       to the geometry's edge collection where control_edge_index is the index
       of the first new edge. Also, num_segments-1 vertices along the arc will
       be appended to the geometry's collection of verttices. In order to
       include an arccurve in a loop the user only needs add the arccurve's
       points to a loop using \ref SULoopInputAddVertexIndex().
@since SketchUp 2017 M2, API 5.2
@param[in]  geom_input         The geometry input object.
@param[in]  start_point        The index of the vertex at the start of the arc.
@param[in]  end_point          The index of the vertex at the end of the arc.
@param[in]  center             The center point of the arc's circle.
@param[in]  normal             The normal vector of the arc plane.
@param[in]  num_segments       The number of edges for the arc.
@param[out] added_curve_index  (optional) If not NULL, returns the index of the
                               added curve.
@param[out] control_edge_index (optional) If not NULL, returns the index of the
                               the arc's control edge which can be used to set
                               the arc's edge properties.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if center or normal is NULL
- \ref SU_ERROR_OUT_OF_RANGE if either start_point or end_point are greater than
       the total number of points in geom_input
- \ref SU_ERROR_INVALID_ARGUMENT if the data specifies an invalid arccurve
*/
SU_RESULT SUGeometryInputAddArcCurve(
    SUGeometryInputRef geom_input, size_t start_point, size_t end_point,
    const struct SUPoint3D* center, const struct SUVector3D* normal, size_t num_segments,
    size_t* added_curve_index, size_t* control_edge_index);

/**
@brief Creates a loop input object.
@param[out] loop_input The object created.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if loop_input is NULL
- \ref SU_ERROR_OVERWRITE_VALID if loop_input already references a valid object
*/
SU_RESULT SULoopInputCreate(SULoopInputRef* loop_input);

/**
@brief Deallocates a loop input object.
@param[in] loop_input The object to deallocate.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if loop_input is NULL
- \ref SU_ERROR_INVALID_INPUT if loop_input points to an invalid object
*/
SU_RESULT SULoopInputRelease(SULoopInputRef* loop_input);

/**
@brief Adds a vertex index to a loop input object.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2017 M2, API 5.2. In previous releases this method returned
         \ref SU_ERROR_INVALID_INPUT if the specified index was already anywhere
         in the loop. In SketchUp 2017 M1 the concept of an explicitly closed
         loop was introduced. A loop can be explicitly closed by either using
         this method to insert an index which is already at the beginning of the
         loop, or by adding a curve to the loop which connects the loop's start
         and end points using \ref SULoopInputAddCurve(). If a loop was not
         previously closed and \ref SULoopInputAddVertexIndex() is used to add
         the loop's start vertex, the loop will be closed and \ref SU_ERROR_NONE
         will be returned. If attempts are made to add vertices after a loop has
         been explicitly closed \ref SU_ERROR_UNSUPPORTED will be returned. If
         an attempt is made to add a vertex that already existed in an open loop
         not at the front \ref SU_ERROR_INVALID_ARGUMENT will be returned.

@param[in] loop_input   The loop input object.
@param[in] vertex_index The vertex index to add. This references a vertex within
                        the parent geometry input's vertex collection (as a
                        zero- based index).
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_UNSUPPORTED if the loop was already closed
- \ref SU_ERROR_INVALID_ARGUMENT if vertex_index already existed in the loop not
       at the front
*/
SU_RESULT SULoopInputAddVertexIndex(SULoopInputRef loop_input, size_t vertex_index);

/**
@brief Sets the hidden flag of an edge in a loop input object.
@param[in] loop_input The loop input object.
@param[in] edge_index The zero-based index of the edge within the loop.
@param[in] hidden     The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current edge
  count
*/
SU_RESULT SULoopInputEdgeSetHidden(SULoopInputRef loop_input, size_t edge_index, bool hidden);

/**
@brief Sets the soft flag of an edge in a loop input object.
@param[in] loop_input The loop input object.
@param[in] edge_index The zero-based index of the edge within the loop.
@param[in] soft       The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current edge
  count
*/
SU_RESULT SULoopInputEdgeSetSoft(SULoopInputRef loop_input, size_t edge_index, bool soft);

/**
@brief Sets the smooth flag of an edge in a loop input object.
@param[in] loop_input The loop input object.
@param[in] edge_index The zero-based index of the edge within the loop.
@param[in] smooth     The flag to set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references beyond the current edge
  count
*/
SU_RESULT SULoopInputEdgeSetSmooth(SULoopInputRef loop_input, size_t edge_index, bool smooth);

/**
@brief Sets the material of an edge in the loop input.
@since SketchUp 2017, API 5.0
@param[in] loop_input The loop input object.
@param[in] edge_index Index of the edge to set the material.
@param[in] material   The material to be set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input or material is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references an edge beyond the total
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
edge count of loop_input
*/
SU_RESULT SULoopInputEdgeSetMaterial(
    SULoopInputRef loop_input, size_t edge_index, SUMaterialRef material);

/**
@brief Sets the layer of an edge in the loop input.
@since SketchUp 2017, API 5.0
@param[in] loop_input The loop input object.
@param[in] edge_index Index of the edge to set the layer.
@param[in] layer      The layer to be set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input or layer is not valid
- \ref SU_ERROR_OUT_OF_RANGE if edge_index references an edge beyond the total
edge count of loop_input
*/
SU_RESULT SULoopInputEdgeSetLayer(SULoopInputRef loop_input, size_t edge_index, SULayerRef layer);

/**
@brief Adds a simple curve to a loop input object.
@param[in] loop_input       The loop input object.
@param[in] first_edge_index First edge index to be associated with the curve.
@param[in] last_edge_index  Last edge index to be associated with the curve.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if first_edge_index or last_edge_index reference an
  edge beyond the loop's total edge count
*/
SU_RESULT SULoopInputAddCurve(
    SULoopInputRef loop_input, size_t first_edge_index, size_t last_edge_index);

/**
@brief Retrieves whether the loop input is closed. A loop input can be closed
       either by re-adding the start vertex to the end of the loop using \ref
       SULoopInputAddVertexIndex or by adding a curve to the loop input which
       connects the loop's start and end points using \ref SULoopInputAddCurve().
@since SketchUp 2017 M2, API 5.2
@param[in]  loop_input The loop input object.
@param[out] is_closed  The flag retrieved (true if the loop is closed).
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if loop_input is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_closed is NULL
  count
*/
SU_RESULT SULoopInputIsClosed(SULoopInputRef loop_input, bool* is_closed);

/**
@brief Adds a face to a geometry input object with a given outer loop for the
       face.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2017 M2, API 5.2. An additional error code was added (\ref
         SU_ERROR_INVALID_ARGUMENT) to indicate to users when the loop contains
         invalid data.

@param[in]  geom_input       The geometry input object.
@param[in]  outer_loop       The outer loop to be set for the face. If the
                             function succeeds (i.e. returns SU_ERROR_NONE),
                             this loop will be deallocated.
@param[out] added_face_index (optional) If not NULL, returns the index of the
                              added face.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if outer_loop is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if the data specifies an invalid loop
*/
SU_RESULT SUGeometryInputAddFace(
    SUGeometryInputRef geom_input, SULoopInputRef* outer_loop, size_t* added_face_index);

/**
@brief Sets a flag in the geometry input that, when true, will create a face by
       reversing the orientations of all of its loops.
@param[in] geom_input The geometry input object.
@param[in] face_index Index of the face to be reversed.
@param[in] reverse    The given reverse flag.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
  face count of geom_input
*/
SU_RESULT SUGeometryInputFaceSetReverse(
    SUGeometryInputRef geom_input, size_t face_index, bool reverse);

/**
@brief Sets the layer of a face in the geometry input.
@param[in] geom_input The geometry input object.
@param[in] face_index Index of the face to be reversed.
@param[in] layer      The layer to be set.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input or layer is not valid
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
  face count of geom_input
*/
SU_RESULT SUGeometryInputFaceSetLayer(
    SUGeometryInputRef geom_input, size_t face_index, SULayerRef layer);

/**
@brief Adds an inner loop to a face in the geometry input.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2017 M2, API 5.2. An additional error code was added (\ref
         SU_ERROR_INVALID_ARGUMENT) to indicate to users when the loop contains
         invalid data.

@param[in] geom_input The geometry input object.
@param[in] face_index Index of the face to receive the inner loop.
@param[in] loop_input The inner loop to be added. If the function succeeds
                      (i.e. returns SU_ERROR_NONE), this loop will be
                      deallocated.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input or *loop_input are not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if loop_input is NULL
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
  face count of geom_input.
- \ref SU_ERROR_INVALID_ARGUMENT if the data specifies an invalid loop
*/
SU_RESULT SUGeometryInputFaceAddInnerLoop(
    SUGeometryInputRef geom_input, size_t face_index, SULoopInputRef* loop_input);

/**
@brief Sets the front material of a face in the geometry input.

@deprecated This function is made obsolete by the newer
            SUGeometryInputFaceSetFrontMaterialPosition() and left in place only
            for compatibility.

@see SUGeometryInputFaceSetFrontMaterialPosition

@param[in] geom_input     The geometry input object.
@param[in] face_index     Index of the face to receive the material.
@param[in] material_input The material input to set.

@related SUGeometryInputRef

@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
  face count of geom_input
- \ref SU_ERROR_NULL_POINTER_INPUT if material_input is NULL
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUGeometryInputFaceSetFrontMaterial(
    SUGeometryInputRef geom_input, size_t face_index, const struct SUMaterialInput* material_input);

/**
@brief Sets the back material of a face in the geometry input.

@deprecated This function is made obsolete by the newer
            SUGeometryInputFaceSetBackMaterialPosition() and left in place only
            for compatibility.

@see SUGeometryInputFaceSetBackMaterialPosition

@param[in] geom_input     The geometry input object.
@param[in] face_index     Index of the face to receive the material.
@param[in] material_input The material input to set.

@related SUGeometryInputRef

@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
  face count of geom_input
- \ref SU_ERROR_NULL_POINTER_INPUT if material_input is NULL
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUGeometryInputFaceSetBackMaterial(
    SUGeometryInputRef geom_input, size_t face_index, const struct SUMaterialInput* material_input);

/**
@brief Sets the front material of a face in the geometry input.

@param[in] geom_input     The geometry input object.
@param[in] face_index     Index of the face to receive the material.
@param[in] material_input The material input to set.

@related SUGeometryInputRef

@since SketchUp 2021.1, API 9.1

@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if \p face_index references a face beyond the total
  face count of \p geom_input
- \ref SU_ERROR_NULL_POINTER_INPUT if \p material_input is NULL
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUGeometryInputFaceSetFrontMaterialByPosition(
    SUGeometryInputRef geom_input, size_t face_index,
    const struct SUMaterialPositionInput* material_input);

/**
@brief Sets the back material of a face in the geometry input.

@param[in] geom_input     The geometry input object.
@param[in] face_index     Index of the face to receive the material.
@param[in] material_input The material input to set.

@related SUGeometryInputRef

@since SketchUp 2021.1, API 9.1

@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if \p face_index references a face beyond the total
  face count of \p geom_input
- \ref SU_ERROR_NULL_POINTER_INPUT if \p material_input is NULL
- \ref SU_ERROR_INVALID_ARGUMENT is the material is owned by a layer or image
*/
SU_RESULT SUGeometryInputFaceSetBackMaterialByPosition(
    SUGeometryInputRef geom_input, size_t face_index,
    const struct SUMaterialPositionInput* material_input);

/**
@brief Sets a flag in the geometry input that, when true, will create a hidden
       face.
@since SketchUp 2017, API 5.0
@param[in] geom_input The geometry input object.
@param[in] face_index Index of the face to be hidden.
@param[in] hidden     The given hidden flag.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if geom_input is not valid
- \ref SU_ERROR_OUT_OF_RANGE if face_index references a face beyond the total
       face count of geom_input
*/
SU_RESULT SUGeometryInputFaceSetHidden(
    SUGeometryInputRef geom_input, size_t face_index, bool hidden);

/**
@brief Returns all the various geometry counts.
@since SketchUp 2018, API 6.0
@param[in]  geom_input      The geometry input object.
@param[out] vertices_count  The total count of vertices.
@param[out] faces_count     The total count of faces.
@param[out] edge_count      The total count of edges.
@param[out] curve_count     The total count of curves.
@param[out] arc_count       The total count of arcs.
@related SUGeometryInputRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if vertices_count, faces_count,
       edge_count, curve_count, or arc_count is NULL
*/
SU_RESULT SUGeometryInputGetCounts(
    SUGeometryInputRef geom_input, size_t* vertices_count, size_t* faces_count, size_t* edge_count,
    size_t* curve_count, size_t* arc_count);

#ifdef __cplusplus
}  // extern "C"
#endif
#pragma pack(pop)

#endif  // SKETCHUP_MODEL_GEOMETRY_INPUT_H_
