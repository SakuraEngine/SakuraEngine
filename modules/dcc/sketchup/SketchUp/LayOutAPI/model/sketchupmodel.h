// Copyright 2015-2023 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_SKETCHUPMODEL_H_
#define LAYOUT_MODEL_SKETCHUPMODEL_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>
#include <SketchUpAPI/model/defs.h>

/**
@struct LOSketchUpModelRef
@brief References a SketchUp model viewport entity.
*/

/**
@enum LOSketchUpModelRenderMode
@brief Defines the different rendering modes available for a SketchUp model.
@note New for LayOut 2023.1, API 8.1 is the value LOSketchUpModelRenderMode_NoOverride.
      The value is only used for the document render mode override, to indicate that
      the output render mode should match the edit render mode, which is set per LOSketchUpModelRef.
*/
typedef enum {
  LOSketchUpModelRenderMode_NoOverride = -1,  ///< Output render mode matches edit render mode.
  LOSketchUpModelRenderMode_Raster = 0,       ///< Raster rendering.
  LOSketchUpModelRenderMode_Hybrid,           ///< Hybrid rendering.
  LOSketchUpModelRenderMode_Vector,           ///< Vector rendering.
  LONumSketchUpModelRenderModes
} LOSketchUpModelRenderMode;

/**
@enum LOSketchUpModelStandardView
@brief Defines the standard views available for a SketchUp model. Relative views
       are not necessarily aligned to the corresponding axes like regular views,
       but rather they maintain the alignment from the previous position of the
       camera.
*/
typedef enum {
  LOSketchUpModelStandardView_Top = 0,      ///< Top. This view is aligned with the X and Y axes.
  LOSketchUpModelStandardView_RelativeTop,  ///< Top (Relative). This top view is oriented relative
                                            ///< to the previous camera position.
  LOSketchUpModelStandardView_Bottom,       ///< Bottom. This view is aligned with the X and Y axes.
  LOSketchUpModelStandardView_RelativeBottom,  ///< Bottom (Relative). This bottom view is oriented
                                               ///< relative to the previous camera position.
  LOSketchUpModelStandardView_Front,  ///< Front. This view is aligned with the X and Z axes.
  LOSketchUpModelStandardView_Back,   ///< Back. This view is aligned with the X and Z axes.
  LOSketchUpModelStandardView_Left,   ///< Left. This view is aligned with the Y and Z axes.
  LOSketchUpModelStandardView_Right,  ///< Right. This view is aligned with the Y and Z axes.
  LOSketchUpModelStandardView_Iso,  ///< Iso. This is aligned with the nearest isometric axes to the
                                    ///< current view.
  LONumSketchUpModelStandardViews
} LOSketchUpModelStandardView;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new SketchUp model object with the specified bounds that
       references the specified SketchUp file.
@param[out] model    The SketchUp model object.
@param[in]  path     The path to the SketchUp file.
@param[in]  bounds   The starting dimensions of the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if model is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *model already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
- \ref SU_ERROR_SERIALIZATION if the .skp file could not be loaded
- \ref SU_ERROR_NO_DATA if the .skp file could not be found
*/
LO_RESULT LOSketchUpModelCreate(
    LOSketchUpModelRef* model, const char* path, const LOAxisAlignedRect2D* bounds);

/**
@brief Adds a reference to a SketchUp model object.
@param[in] model The SketchUp model object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
*/
LO_RESULT LOSketchUpModelAddReference(LOSketchUpModelRef model);

/**
@brief Releases a SketchUp model object. The object will be invalidated if
       releasing the last reference.
@param[in] model The SketchUp model object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT model is NULL
- \ref SU_ERROR_INVALID_INPUT *model does not refer to a valid object
*/
LO_RESULT LOSketchUpModelRelease(LOSketchUpModelRef* model);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOSketchUpModelRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOSketchUpModelRef.
@param[in] entity The entity object.
@return
- The converted \ref LOSketchUpModelRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOSketchUpModelRef LOSketchUpModelFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOSketchUpModelRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] model The SketchUp model object.
@return
- The converted \ref LOEntityRef if model is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOSketchUpModelToEntity(LOSketchUpModelRef model);

/**
@brief Gets the SUModelRef representation of the SketchUp model.
@since LayOut 2017, API 2.0
@param[in]  model   The SketchUp model object.
@param[out] sumodel The model reference object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if sumodel is NULL
*/
LO_RESULT LOSketchUpModelGetModel(LOSketchUpModelRef model, SUModelRef* sumodel);

/**
@brief Gets the render mode of the SketchUp model.
@param[in]  model       The SketchUp model object.
@param[out] render_mode The render mode of the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if render_mode is NULL
*/
LO_RESULT LOSketchUpModelGetRenderMode(
    LOSketchUpModelRef model, LOSketchUpModelRenderMode* render_mode);

/**
@brief Sets the render mode of the SketchUp model.
@param[in] model       The SketchUp model object.
@param[in] render_mode The new render mode for the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if the value of render_mode is invalid
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetRenderMode(
    LOSketchUpModelRef model, LOSketchUpModelRenderMode render_mode);

/**
 @brief Renders the SketchUp model. If the model belongs to a
        \ref LODocumentRef, then the render will be performed at the
        resolution set in document.page_info (see \ref LODocumentRef and
        \ref LOPageInfoRef). Otherwise, the render will be performed at Low
        resolution.
@param[in] model The SketchUp model object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelRender(LOSketchUpModelRef model);

/**
@brief Gets the status indicating whether the SketchUp model needs to be
       rendered.
@param[in]  model         The SketchUp model object.
@param[out] render_needed True if the SketchUp model needs to be rendered.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if render_needed is NULL
*/
LO_RESULT LOSketchUpModelIsRenderNeeded(LOSketchUpModelRef model, bool* render_needed);

/**
@brief Gets the number of scenes that are available for a SketchUp model. Use
       this to determine the size of the array that should be passed to \ref
       LOSketchUpModelGetAvailableScenes.
@param[in]  model            The SketchUp model object.
@param[out] number_of_scenes The number of scenes.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if number_of_scenes is NULL
*/
LO_RESULT LOSketchUpModelGetNumberOfAvailableScenes(
    LOSketchUpModelRef model, size_t* number_of_scenes);

/**
@brief Gets the array of scenes that are available for a SketchUp model. The
       first scene returned will always be the default scene, called
       "Last saved SketchUp View".
@param[in]  model  The SketchUp model object.
@param[in]  len    The maximum number of scenes to retrieve.
@param[out] scenes The names of the scenes retrieved.
@param[out] count  The number of scenes retrieved.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scenes or count is NULL
*/
LO_RESULT LOSketchUpModelGetAvailableScenes(
    LOSketchUpModelRef model, size_t len, SUStringRef scenes[], size_t* count);

/**
@brief Gets the most recently selected scene of the SketchUp model.
@param[in]  model          The SketchUp model object.
@param[out] scene_index    The index of the SketchUp model's scene. This is an
                           index into the list of available scenes returned by
                           \ref LOSketchUpModelGetAvailableScenes.
@param[out] scene_modified Indicates whether or not the scene has been modified
                           in LayOut.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scene_index or scene_modified is NULL
- \ref SU_ERROR_NO_DATA if the most recently selected scene no longer exists
*/
LO_RESULT LOSketchUpModelGetCurrentScene(
    LOSketchUpModelRef model, size_t* scene_index, bool* scene_modified);

/**
@brief Sets the scene of the SketchUp model.
@param[in] model       The SketchUp model object.
@param[in] scene_index The index of the scene. This is an index into the list
                       of available scenes returned by \ref
                       LOSketchUpModelGetAvailableScenes.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if scene_index is greater than or equal to the
 number of available scenes.
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked

*/
LO_RESULT LOSketchUpModelSetCurrentScene(LOSketchUpModelRef model, size_t scene_index);

/**
@brief Gets whether the SketchUp model's camera has been modified.
@param[in]  model    The SketchUp model object.
@param[out] modified Indicates whether or not the camera has been modified
                     in LayOut.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if modified is NULL
*/
LO_RESULT LOSketchUpModelGetCameraModified(LOSketchUpModelRef model, bool* modified);

/**
@brief Resets the SketchUp model's camera to the scene's setting.
@param[in] model The SketchUp model object.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelResetCamera(LOSketchUpModelRef model);

/**
@brief Gets whether the SketchUp model's shadow or fog effects have been modified.
@param[in]  model    The SketchUp model object.
@param[out] modified Indicates whether or not the shadow or fog effects have
                     been modified in LayOut.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if modified is NULL
*/
LO_RESULT LOSketchUpModelGetEffectsModified(LOSketchUpModelRef model, bool* modified);

/**
@brief Resets the SketchUp model's shadow and fog effects to the scene's settings.
@param[in] model The SketchUp model object.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelResetEffects(LOSketchUpModelRef model);

/**
@brief Gets whether the SketchUp model's style has been modified.
@param[in]  model    The SketchUp model object.
@param[out] modified Indicates whether or not the style has been modified in
                     LayOut.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if modified is NULL
*/
LO_RESULT LOSketchUpModelGetStyleModified(LOSketchUpModelRef model, bool* modified);

/**
@brief Resets the SketchUp model's style to the scene's setting.
@param[in] model The SketchUp model object.
@since LayOut 2020.1, API 5.1
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelResetStyle(LOSketchUpModelRef model);

/**
@brief Gets whether the SketchUp model's layers have been modified.
@note  In SketchUp 2020, SketchUp "layers" were renamed to "tags". For
       consistency with the SketchUp API, this will continue to refer to "tags"
       as "layers".
@since LayOut 2020.1, API 5.1
@param[in]  model    The SketchUp model object.
@param[out] modified Indicates whether or not the tag have been modified in
                     LayOut.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if modified is NULL
*/
LO_RESULT LOSketchUpModelGetLayersModified(LOSketchUpModelRef model, bool* modified);

/**
@brief Resets the SketchUp model's layers to the scene's setting.
@note  In SketchUp 2020, SketchUp "layers" were renamed to "tags". For
       consistency with the SketchUp API, this will continue to refer to "tags"
       as "layers".
@since LayOut 2020.1, API 5.1
@param[in] model The SketchUp model object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelResetLayers(LOSketchUpModelRef model);

/**
@brief Gets the status of whether or not the background is displayed for the
       SketchUp model. This setting only applies when the render mode is
       \ref LOSketchUpModelRenderMode_Vector.
@param[in]  model              The SketchUp model object.
@param[out] display_background True if the background is displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if display_background is NULL
*/
LO_RESULT LOSketchUpModelGetDisplayBackground(LOSketchUpModelRef model, bool* display_background);

/**
@brief Sets the status of whether or not the background is displayed for the
       SketchUp model. This setting only applies when the render mode is
       \ref LOSketchUpModelRenderMode_Vector.
@param[in] model              The SketchUp model object.
@param[in] display_background True if the background should be displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetDisplayBackground(LOSketchUpModelRef model, bool display_background);

/**
@brief Gets the line weight in points for the SketchUp model.
@param[in]  model       The SketchUp model object.
@param[out] line_weight Line weight in points.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if line_weight is NULL
*/
LO_RESULT LOSketchUpModelGetLineWeight(LOSketchUpModelRef model, double* line_weight);

/**
@brief Sets the line weight in points for the SketchUp model.
@param[in] model       The SketchUp model object.
@param[in] line_weight Line weight in points.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if line_weight is less than 0.01
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetLineWeight(LOSketchUpModelRef model, double line_weight);

/**
@brief Gets the scale for dashes in the SketchUp model. A scale value of 0.0
       means the dashes are scaled based on the line weight.
@since LayOut 2019, API 4.0
@param[in]  model      The SketchUp model object.
@param[out] dash_scale Dash scale. Will be in the range [0, 10].
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dash_scale is NULL
*/
LO_RESULT LOSketchUpModelGetDashScale(LOSketchUpModelRef model, double* dash_scale);

/**
@brief Sets the scale for dashes in the SketchUp model. A scale value of 0.0 or
       lower will "auto" scale the dashes based on the line weight.
@since LayOut 2019, API 4.0
@param[in] model      The SketchUp model object.
@param[in] dash_scale Dash scale. A value less than zero will be set to 0.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetDashScale(LOSketchUpModelRef model, double dash_scale);

/**
@brief Gets whether or not the view is rendered in perspective mode.
@param[in]  model       The SketchUp model object.
@param[out] perspective Whether or not the view is rendered in perspective mode.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if perspective is NULL
*/
LO_RESULT LOSketchUpModelGetPerspective(LOSketchUpModelRef model, bool* perspective);

/**
@brief Sets whether or not the view is rendered in perspective mode.
@param[in] model       The SketchUp model object.
@param[in] perspective Whether or not the view should be rendered in
                       perspective mode.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetPerspective(LOSketchUpModelRef model, bool perspective);

/**
@brief Gets the standard view that a SketchUp model is currently set to.
@param[in]  model The SketchUp model object.
@param[out] view  The standard view that the SketchUp model is currently set to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if view is NULL
- \ref SU_ERROR_NO_DATA if the SketchUp model is not currently set to a
  standard view
*/
LO_RESULT LOSketchUpModelGetStandardView(
    LOSketchUpModelRef model, LOSketchUpModelStandardView* view);

/**
@brief Sets a SketchUp model to use a standard view.
@param[in] model The SketchUp model object.
@param[in] view  The standard view to use.
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
- \ref SU_ERROR_OUT_OF_RANGE if view specifies an invalid value
*/
LO_RESULT LOSketchUpModelSetStandardView(
    LOSketchUpModelRef model, LOSketchUpModelStandardView view);

/**
@brief Gets the scale of a SketchUp model. This is only valid to call for
       models that are rendered in orthographic (non-perspective) mode.
@param[in]  model The SketchUp model object.
@param[out] scale The SketchUp model's scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scale is NULL
- \ref SU_ERROR_NO_DATA if model is currently rendered in perspective mode.
*/
LO_RESULT LOSketchUpModelGetScale(LOSketchUpModelRef model, double* scale);

/**
@brief Sets the scale of a SketchUp model. This is only valid to call for
       models that are rendered in orthographic (non-perspective) mode.
@param[in] model The SketchUp model object.
@param[in] scale The new scale for the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_GENERIC if model is rendered in perspective mode
- \ref SU_ERROR_OUT_OF_RANGE if scale is less than 0.0000001
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetScale(LOSketchUpModelRef model, double scale);

/**
@brief Gets whether or not the scale is preserved when the SketchUp model is
       resized.
@param[in]  model          The SketchUp model object.
@param[out] preserve_scale True if the scale is preserved on resize.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if preserve_scale is NULL
*/
LO_RESULT LOSketchUpModelGetPreserveScaleOnResize(LOSketchUpModelRef model, bool* preserve_scale);

/**
@brief Sets whether or not the scale is preserved when the SketchUp model is
       resized.
@param[in] model          The SketchUp model object.
@param[in] preserve_scale True if the scale should be preserved on resize.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetPreserveScaleOnResize(LOSketchUpModelRef model, bool preserve_scale);

/**
@brief Converts a 3D model space point in the SketchUp model to a 2D paper
       space point on the page.
@param[in] model        The SketchUp model object.
@param[in] model_point  The 3D model space point.
@param[out] paper_point The 2D paper space point.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if model_point is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if paper_point is NULL
*/
LO_RESULT LOSketchUpModelConvertModelPointToPaperPoint(
    LOSketchUpModelRef model, const LOPoint3D* model_point, LOPoint2D* paper_point);

/**
@brief Returns any clip mask assigned to the SketchUp model.
@since LayOut 2017, API 2.0
@param[in]  model     The SketchUp model object.
@param[out] clip_mask The clip mask of the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if clip_mask is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *clip_mask already refers to a valid object
- \ref SU_ERROR_NO_DATA if the model is not being clipped by a clip mask
*/
LO_RESULT LOSketchUpModelGetClipMask(LOSketchUpModelRef model, LOEntityRef* clip_mask);

/**
@brief Sets the clip mask of the SketchUp model. A clip mask defines a region
       of the entity that is visible. This allows you to crop with arbitrary
       shapes. This operation will replace any clip mask that is already
       assigned to this model. The entity being used must not be already part
       of a document or group. The clip mask entity must be either a rectangle,
       ellipse or a path.
@note  Starting in LayOut 2020.1, API 5.1, clip_mask may be SU_INVALID, which
       will remove the existing clip mask, if any.
@since LayOut 2017, API 2.0
@param[in] model     The SketchUp model object.
@param[in] clip_mask The new clip mask for the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_GENERIC if clip_mask is already in a document or group
- \ref SU_ERROR_UNSUPPORTED if clip_mask is not a rectangle, ellipse, or path
- \ref SU_ERROR_LAYER_LOCKED if model is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if model is locked
*/
LO_RESULT LOSketchUpModelSetClipMask(LOSketchUpModelRef model, LOEntityRef clip_mask);

/**
@brief Creates the entities that represent the SketchUp model in its exploded
       form and adds them to a \ref LOEntityListRef. It is NOT necessary to
       explicitly release these entities, since \ref LOEntityListRef itself
       adds a reference to the entities and will release them when they are
       removed from the list or when the list is released. NOTE The behavior
       of this method changed in API 3.0 - an exploded raster-rendered model
       will now wrap the \ref LOImageRef in a \ref LOGroupRef.
@param[in] model       The SketchUp model object.
@param[in] entity_list The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOSketchUpModelGetExplodedEntities(LOSketchUpModelRef model, LOEntityListRef entity_list);

/**
@brief Creates the entities that represent the SketchUp model using its output settings and adds
them to a \ref LOEntityListRef.

This will use the document's output render mode override setting for SketchUp model viewports, and
the raster output image quality set by \ref LOPageInfoSetOutputResolution. It is NOT necessary to
explicitly release these entities, since \ref LOEntityListRef itself adds a reference to the
entities and will release them when they are removed from the list or when the list is released.
@since Layout 2023.1, API 8.1
@param[in] model       The SketchUp model object.
@param[in] entity_list The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p model does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if \p entity_list does not refer to a valid object
*/
LO_RESULT LOSketchUpModelGetOutputEntities(LOSketchUpModelRef model, LOEntityListRef entity_list);
#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_SKETCHUPMODEL_H_
