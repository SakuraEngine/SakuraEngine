// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUEntitiesRef.
 */
#ifndef SKETCHUP_MODEL_ENTITIES_H_
#define SKETCHUP_MODEL_ENTITIES_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/curve.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUEntitiesRef
@brief References a container object for all entities in a model,
       component definition or a group.
*/

/**
@brief Removes all entities in the container.
@since SketchUp 2019, API 7.0
@param[in] entities The entities to clear.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
*/
SU_RESULT SUEntitiesClear(SUEntitiesRef entities);

/**
@brief SUEntitiesFill is the fastest way to populate an entities object. The
       important precondition is that no duplicate data should be given.

NOTE: Faces included in the geometry input object will be merged together when
  using this function. This only applies to geometry in the geometry input
  object and not to any already-existing geometry in the entities object.
  Examples of merging are:
- If weld_vertices is true, duplicated vertices are merged.
- Coincident faces are merged.
- Coincident faces with opposite normals are merged into a single face using the
  appropriate materials from both faces as the front and back materials.
- Faces are created from coplanar edge loops.
- Conincident edges are merged. Visibility is retained when visible and
  invisible edges are welded together. Hardness is retained when hard and soft
  edges are welded together.

@param[in] entities      The entities to populate. Must be an empty entities
                         object.
@param[in] geom_input    The geometry input that the entities object is to be
                         populated with.
@param[in] weld_vertices Flag indicating whether to join coincident vertices.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities or geom_input are invalid objects
*/
SU_RESULT SUEntitiesFill(SUEntitiesRef entities, SUGeometryInputRef geom_input, bool weld_vertices);

/**
@brief Retrieves the bounding box of the entities.
@param[in]  entities The entities object.
@param[out] bbox     The bounding box retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bbox is NULL
*/
SU_RESULT SUEntitiesGetBoundingBox(SUEntitiesRef entities, struct SUBoundingBox3D* bbox);

/**
@brief Retrieves the LLA coordinates (Latidue, Longitude and Altitude) bounding
       box of the given entities object.
       Note that the altitude is calculated based on the model origin, Example:
       If an entities object has a bounding box with the following values
       {{100,100,100}, {200,200,200}} the result will be something like the
       following: {{Latitude, Longitude, 100/METERS_TO_INCHES},
       {Latitude, Longitude, 200/METERS_TO_INCHES}} where Latitude and Longitude
       are the geographical coordinates and altitude is just a conversion from
       inches to meters.
@since SketchUp 2018 M0, API 6.0
@param[in]  entities The entities object.
@param[out] bbox     The latidue longitude and altitude bounding box retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NO_DATA if entities doesn't belong to a valid model.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bbox is NULL
*/
SU_RESULT SUEntitiesGetBoundingBoxLLA(SUEntitiesRef entities, struct SUBoundingBox3D* bbox);

/**
@brief Retrieves the number of faces in the entities object.
@param[in] entities The entities object.
@param[out] count   The number of faces.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumFaces(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the faces in the entities object.
@param[in]  entities The entities object.
@param[in]  len      The number of faces to retrieve.
@param[out] faces    The faces retrieved.
@param[out] count    The number of faces retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if faces or count is NULL
*/
SU_RESULT SUEntitiesGetFaces(SUEntitiesRef entities, size_t len, SUFaceRef faces[], size_t* count);

/**
@brief Retrieves the number of curves in the entities object that are not
       associated with a face.
@param[in]  entities The entities object.
@param[out] count    The number of curves.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumCurves(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the curves in the entities object that are not associated with
       a face.
@param[in]  entities The entities object.
@param[in]  len      The number of curves to retrieve.
@param[out] curves   The curves retrieved.
@param[out] count    The number of curves retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if curves or count is NULL
*/
SU_RESULT SUEntitiesGetCurves(
    SUEntitiesRef entities, size_t len, SUCurveRef curves[], size_t* count);

/**
@brief Retrieves the number of arccurves in the entities object that are not
       associated with a face.
@since SketchUp 2016, API 4.0
@param[in]  entities The entities object.
@param[out] count    The number of arccurves.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumArcCurves(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the arccurves in the entities object that are not associated
       with a face.
@since SketchUp 2016, API 4.0
@param[in]  entities    The entities object.
@param[in]  len         The number of arccurves to retrieve.
@param[out] arccurves   The arccurves retrieved.
@param[out] count       The number of curves retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if arccurves or count is NULL
*/
SU_RESULT SUEntitiesGetArcCurves(
    SUEntitiesRef entities, size_t len, SUArcCurveRef arccurves[], size_t* count);

/**
@brief Retrieves the number of guide points in the entities object.
@since SketchUp 2014 M1, API 2.1
@param[in]  entities The entities object.
@param[out] count    The number of guide_points.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumGuidePoints(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the guide points in the entities object.
@since SketchUp 2014 M1, API 2.1
@param[in]  entities     The entities object.
@param[in]  len          The number of guide points to retrieve.
@param[out] guide_points The guide_points retrieved.
@param[out] count        The number of guide_points retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guide_points or count is NULL
*/
SU_RESULT SUEntitiesGetGuidePoints(
    SUEntitiesRef entities, size_t len, SUGuidePointRef guide_points[], size_t* count);

/**
@brief Retrieves the number of guide lines in the entities object.
@since SketchUp 2016, API 4.0
@param[in]  entities The entities object.
@param[out] count    The number of guide_lines.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumGuideLines(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the guide lines in the entities object.
@since SketchUp 2016, API 4.0
@param[in]  entities    The entities object.
@param[in]  len         The number of guide lines to retrieve.
@param[out] guide_lines The guide_lines retrieved.
@param[out] count       The number of guide_lines retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guide_lines or count is NULL
*/
SU_RESULT SUEntitiesGetGuideLines(
    SUEntitiesRef entities, size_t len, SUGuideLineRef guide_lines[], size_t* count);

/**
@brief Retrieves the number of edges in the entities object.
@param[in]  entities        The entities object.
@param[in]  standalone_only Whether to count all edges (false) or only the edges
                            not attached to curves and faces (true).
@param[out] count           The number of edges.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumEdges(SUEntitiesRef entities, bool standalone_only, size_t* count);

/**
@brief Retrieves the edges in the entities object.
@param[in]  entities        The entities object.
@param[in]  standalone_only Whether to get all edges (false) or only the edges
                            not attached to curves and faces (true).
@param[in]  len             The number of edges to retrieve.
@param[out] edges           The edges retrieved.
@param[out] count           The number of edges retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if edges or count is NULL
*/
SU_RESULT SUEntitiesGetEdges(
    SUEntitiesRef entities, bool standalone_only, size_t len, SUEdgeRef edges[], size_t* count);

/**
@brief Retrieves the number of polyline3d's in the entities object.
@param[in]  entities The entities object.
@param[out] count    The the number of polyline3d's.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumPolyline3ds(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the polyline3d's in the entities object.
@param[in]  len      The number of polyline3d's to retrieve.
@param[in]  entities The entities object.
@param[out] lines    The polyline3d's retrieved.
@param[out] count    The number of polyline3d's retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if lines or count is NULL
*/
SU_RESULT SUEntitiesGetPolyline3ds(
    SUEntitiesRef entities, size_t len, SUPolyline3dRef lines[], size_t* count);

/**
@brief Adds face objects to a entities object.

@bug SUEntitiesAddFaces() will not merge overlapping vertices and edges, which
  produces SketchUp models with unexpected state. Avoid using these functions
  and instead use SUGeometryInputRef along with SUEntitiesFill().

@param[in] entities The entities object.
@param[in] len      The length of the array of face objects.
@param[in] faces    The array of face objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if faces is NULL
*/
SU_RESULT SUEntitiesAddFaces(SUEntitiesRef entities, size_t len, const SUFaceRef faces[]);

/**
@brief Adds edge objects to an entities object.

NOTE: This function does not merge geometry, which will likely create an invalid
SketchUp model. It is recommended to use SUGeometryInput instead which does
correctly merge geometry.

@param[in] entities The entities object.
@param[in] len      The length of the array of edge objects.
@param[in] edges    The array of edge objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if edges is NULL
*/
SU_RESULT SUEntitiesAddEdges(SUEntitiesRef entities, size_t len, const SUEdgeRef edges[]);

/**
@brief Adds curve objects to an entities object.

NOTE: This function does not merge geometry, which will likely create an invalid
SketchUp model. It is recommended to use SUGeometryInput instead which does
correctly merge geometry.

@param[in] entities The entities object.
@param[in] len      The length of the array of curve objects.
@param[in] curves   The array of curve objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if curves is NULL
*/
SU_RESULT SUEntitiesAddCurves(SUEntitiesRef entities, size_t len, const SUCurveRef curves[]);

/**
@brief Adds arccurve objects to an entities object.
@since SketchUp 2016, API 4.0

NOTE: This function does not merge geometry, which will likely create an invalid
SketchUp model. It is recommended to use SUGeometryInput instead which does
correctly merge geometry.

@param[in] entities The entities object.
@param[in] len      The length of the array of curve objects.
@param[in] curves   The array of arccurve objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if curves is NULL
*/
SU_RESULT SUEntitiesAddArcCurves(SUEntitiesRef entities, size_t len, const SUArcCurveRef curves[]);

/**
@brief Adds guide point objects to an entities object.
@since SketchUp 2014 M1, API 2.1
@param[in] entities     The entities object.
@param[in] len          The length of the array of guide point objects.
@param[in] guide_points The array of guide point objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_points is NULL
*/
SU_RESULT SUEntitiesAddGuidePoints(
    SUEntitiesRef entities, size_t len, const SUGuidePointRef guide_points[]);

/**
@brief Adds guide line objects to an entities object.
@since SketchUp 2016, API 4.0
@param[in] entities    The entities object.
@param[in] len         The length of the array of guide line objects.
@param[in] guide_lines The array of guide line objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if guide_lines is NULL
*/
SU_RESULT SUEntitiesAddGuideLines(
    SUEntitiesRef entities, size_t len, const SUGuideLineRef guide_lines[]);

/**
@brief Adds a group object to an entities object.
@param[in] entities The entities object.
@param[in] group    The group object to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities or group is not a valid object
*/
SU_RESULT SUEntitiesAddGroup(SUEntitiesRef entities, SUGroupRef group);

/**
@brief Adds an image object to an entities object.
@param[in] entities The entities object.
@param[in] image    The image object to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities or image is not a valid object
*/
SU_RESULT SUEntitiesAddImage(SUEntitiesRef entities, SUImageRef image);

/**
@brief Adds a component instance object to the entities.
@param[in]  entities The entities object.
@param[in]  instance The component instance object to add.
@param[out] name     The unique name that is assigned to definition of the
                     component instance. This can be NULL in which case the
                     caller does not need to retrieve the assigned name.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities or instance is not a valid object
- \ref SU_ERROR_INVALID_OUTPUT if name (when not NULL) does not refer to a valid
  \ref SUStringRef object
*/
SU_RESULT SUEntitiesAddInstance(
    SUEntitiesRef entities, SUComponentInstanceRef instance, SUStringRef* name);

/**
@brief Adds section plane objects to an entities object.
@since SketchUp 2016, API 4.0
@param[in] entities        The entities object.
@param[in] len             The length of the array of section planes objects.
@param[in] section_planes  The array of section planes objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if section_planes is NULL
*/
SU_RESULT SUEntitiesAddSectionPlanes(
    SUEntitiesRef entities, size_t len, const SUSectionPlaneRef section_planes[]);

/**
@brief Adds text objects to an entities object.
@since SketchUp 2018, API 6.0
@param[in] entities The entities object.
@param[in] len      The length of the array of text objects.
@param[in] texts    The array of text objects to add.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if texts is NULL
*/
SU_RESULT SUEntitiesAddTexts(SUEntitiesRef entities, size_t len, const SUTextRef texts[]);

/**
@brief Retrieves the number of groups in the entities.
@param[in]  entities The entities object.
@param[out] count    The number of groups.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is an invalid entities object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumGroups(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the groups in the entities.
@param[in]  entities The entities object.
@param[in]  len      The number of groups to retrieve.
@param[out] groups   The groups retrieved.
@param[out] count    The number of groups retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if groups or count is NULL
*/
SU_RESULT SUEntitiesGetGroups(
    SUEntitiesRef entities, size_t len, SUGroupRef groups[], size_t* count);

/**
@brief Retrieves the number of images in the entities.
@param[in]  entities  The entities object.
@param[out] count     The number of image objects.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumImages(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the array of image objects of a entities object.
@param[in]  entities The entities object.
@param[in]  len      The number of image objects to retrieve.
@param[out] images   The image objects retrieved.
@param[out] count    The number of image objects retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if images or count is NULL
*/
SU_RESULT SUEntitiesGetImages(
    SUEntitiesRef entities, size_t len, SUImageRef images[], size_t* count);

/**
@brief Retrieves the number of component instances in the entities.
@param[in] entities The entities object.
@param[out] count   The number of component instances.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is an invalid entities object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumInstances(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the component instances in the entities.
@param[in]  entities  The entities object.
@param[in]  len       The number of component instances to retrieve.
@param[out] instances The component instances retrieved.
@param[out] count     The number of component instances retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instances or count is NULL
*/
SU_RESULT SUEntitiesGetInstances(
    SUEntitiesRef entities, size_t len, SUComponentInstanceRef instances[], size_t* count);

/**
@brief Retrieves the number of section planes in the entities object.
@since SketchUp 2016, API 4.0
@param[in]  entities The entities object.
@param[out] count    The number of section planes.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumSectionPlanes(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the number of texts in the entities object.
@since SketchUp 2018, API 6.0
@param[in]  entities The entities object.
@param[out] count    The number of texts.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumTexts(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the section planes in the entities.
@since SketchUp 2016, API 4.0
@param[in]  entities       The entities object.
@param[in]  len            The number of section planes to retrieve.
@param[out] section_planes The section planes retrieved.
@param[out] count          The number of section planes retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if section_planes or count is NULL
*/
SU_RESULT SUEntitiesGetSectionPlanes(
    SUEntitiesRef entities, size_t len, SUSectionPlaneRef section_planes[], size_t* count);

/**
@brief Retrieves the number of dimensions in the entities object.
@since SketchUp 2017, API 5.0
@param[in]  entities The entities object.
@param[out] count    The number of dimensions.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUEntitiesGetNumDimensions(SUEntitiesRef entities, size_t* count);

/**
@brief Retrieves the dimensions in the entities object.
@since SketchUp 2017, API 5.0
@param[in]  entities   The entities object.
@param[in]  len        The number of dimensions to retrieve.
@param[out] dimensions The dimensions retrieved.
@param[out] count      The number of dimensions retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dimensions or count is NULL
*/
SU_RESULT SUEntitiesGetDimensions(
    SUEntitiesRef entities, size_t len, SUDimensionRef* dimensions, size_t* count);

/**
@brief Retrieves the texts in the entities.
@since SketchUp 2018, API 6.0
@param[in]  entities The entities object.
@param[in]  len      The number of section planes to retrieve.
@param[out] texts    The texts retrieved.
@param[out] count    The number of texts retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if texts or count is NULL
*/
SU_RESULT SUEntitiesGetTexts(SUEntitiesRef entities, size_t len, SUTextRef texts[], size_t* count);

/**
@brief Applies a 3D transformation to the elements of the provided entity array.
@since SketchUp 2017, API 5.0
@param[in] entities The entities object.
@param[in] len      The number of entities in the array.
@param[in] elements The elements to be transformed.
@param[in] trans    The transform to be applied.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if trans or elements are NULL
- \ref SU_ERROR_UNSUPPORTED if any of the elements in the array are not
       contained by entities
- \ref SU_ERROR_GENERIC if the transformation operation fails
*/
SU_RESULT SUEntitiesTransform(
    SUEntitiesRef entities, size_t len, SUEntityRef elements[],
    const struct SUTransformation* trans);

/**
@brief Applies a 3D transformations to the elements of the provided entity
       array. The arrays of elements and transformations must be the same
       length.
@since SketchUp 2017, API 5.0
@param[in] entities  The entities object.
@param[in] len       The number of entities in the array.
@param[in] elements  The elements to be transformed.
@param[in] tranforms The transformations to be applied.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if elements or tranforms are NULL
- \ref SU_ERROR_UNSUPPORTED if any of the elements in the array are not
       contained by entities
- \ref SU_ERROR_GENERIC if the transformation operation fails
*/
SU_RESULT SUEntitiesTransformMultiple(
    SUEntitiesRef entities, size_t len, SUEntityRef elements[],
    const struct SUTransformation tranforms[]);

/**
@brief Erases elements from an entities object. The input elements are
       destroyed, so the array elements are invalidated to prevent user from
       attempting to use destroyed entities.
@since SketchUp 2017, API 5.0
@param[in] entities The entities object.
@param[in] len      The number of entities in the array.
@param[in] elements The elements to be destroyed.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if elements is NULL
- \ref SU_ERROR_UNSUPPORTED if any of the elements in the array are not
       contained by entities
*/
SU_RESULT SUEntitiesErase(SUEntitiesRef entities, size_t len, SUEntityRef elements[]);

/**
@brief Retrieves a boolean indicating whether the entities object is
       recursively empty. A recursively empty entities object is defined as one
       that either has zero entities or contains only instances of definitions
       with recursively empty entities objects.
@since SketchUp 2017, API 5.0
@param[in]  entities The entities object.
@param[out] is_empty The bool value retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_empty is NULL
*/
SU_RESULT SUEntitiesIsRecursivelyEmpty(SUEntitiesRef entities, bool* is_empty);

/**
@brief Retrieves a boolean by recursively searching through the entities
       determining whether the entities has an active section plane or any
       of its nested components have an active section plane.
@since SketchUp 2018, API 6.0
@param[in]  entities          The entities object.
@param[out] has_section_cuts  The bool value retrieved.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if has_section_cuts is NULL
*/
SU_RESULT SUEntitiesHasSectionCuts(SUEntitiesRef entities, bool* has_section_cuts);

/**
@brief Fills the list with all entities of the specified type in the instance.
       The list is not in any specific order.
@since SketchUp 2018, API 6.0
@param[in]  entities The entities object to be queried.
@param[in]  type     The type of entities to be collected.
@param[out] list     The list object to be filled.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_INVALID_OUTPUT if list is not a valid object
*/
SU_RESULT SUEntitiesEntityListFill(
    SUEntitiesRef entities, enum SURefType type, SUEntityListRef list);

/**
@brief Takes a set of edges and find all possible chains of edges and connect
       them with a \ref SUCurveRef.

       A curve will not cross another curve. They will split where multiple
       curves meet.

@since SketchUp 2020.1, API 8.1
@param[in]  entities   The entities object to be queried.
@param[in]  num_edges  The length of the array of edge objects.
@param[in]  edges      The array of edge objects to weld.
@param[out] list       The list object to be filled with \ref SUCurveRef objects.
@related SUEntitiesRef
@see SUArcCurveRef
@see SUCurveRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if edges is NULL
- \ref SU_ERROR_INVALID_OUTPUT if list is not a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if edges contains edges that don't belong to
         the same entities collection.
*/
SU_RESULT SUEntitiesWeld(
    SUEntitiesRef entities, size_t num_edges, SUEdgeRef edges[], SUEntityListRef list);

/**
@brief Reference to the parent of an SUEntitiesRef object.
@since SketchUp 2021.1, API 9.1
*/
struct SUEntitiesParent {
  /// The parent model, or \p SU_INVALID if the parent is a component definition.
  SUModelRef model;

  /// The parent component definition, or \p SU_INVALID if the parent is a model.
  SUComponentDefinitionRef definition;
};

/**
@brief Get the parent component definition or model that owns this entities object.
@since SketchUp 2021.1, API 9.1
@param[in]  entities
@param[out] parent
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p entities is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p parent is NULL.
*/
SU_RESULT SUEntitiesGetParent(SUEntitiesRef entities, struct SUEntitiesParent* parent);

/**
@brief Get the active section plane for this entities object.
@since SketchUp 2021.1, API 9.1
@param[in]  entities
@param[out] section_plane
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success.
- \ref SU_ERROR_INVALID_INPUT if \p entities is not a valid object.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p entities is NULL.
- \ref SU_ERROR_OVERWRITE_VALID if \p section_plane already references a valid object.
- \ref SU_ERROR_NO_DATA if \p entities has no active section plane.
 */
SU_RESULT SUEntitiesGetActiveSectionPlane(SUEntitiesRef entities, SUSectionPlaneRef* section_plane);

/**
@brief Set the active section plane for this entities object.
@since SketchUp 2021.1, API 9.1
@bug Before SketchUp 2023.1 (API 11.1) this function would not accept SU_INVALID as a valid
        \p section_plane value and would have returned SU_ERROR_INVALID_INPUT instead
@param[in] entities
@param[in] section_plane The section plane to activate or \p SU_INVALID if none should be active.
@related SUEntitiesRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p entities is not a valid object.
- \ref SU_ERROR_INVALID_ARGUMENT if \p section_plane doesn't belong to \p entities.
 */
SU_RESULT SUEntitiesSetActiveSectionPlane(SUEntitiesRef entities, SUSectionPlaneRef section_plane);

#ifdef __cplusplus
}  // extern "C"
#endif
#pragma pack(pop)

#endif  // SKETCHUP_MODEL_ENTITIES_H_
