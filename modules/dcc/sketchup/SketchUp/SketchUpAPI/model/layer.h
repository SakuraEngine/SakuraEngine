// Copyright 2013-2019 Trimble, Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SULayerRef.
 */
#ifndef SKETCHUP_MODEL_LAYER_H_
#define SKETCHUP_MODEL_LAYER_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULayerRef
@extends SUEntityRef
@brief References a Tag object.
@note 'Layer' is a legacy term that is being used for consistency within the API.
*/

/**
 @enum SULayerVisibilityDefaultType
 @brief Indicates whether to set the layer visible by default. Used for
 \ref SULayerGetSceneBehavior() and \ref SULayerSetSceneBehavior().
 */
enum SULayerVisibilityDefaultType {
  SULayerVisibilityDefaultType_Visible = 0x0000,
  SULayerVisibilityDefaultType_Hidden = 0x0001
};

/**
 @enum SULayerVisibilityNewSceneType
 @brief Indicates whether to set the layer visible on new scenes. Used for
 \ref SULayerGetSceneBehavior() and \ref SULayerSetSceneBehavior().
 */
enum SULayerVisibilityNewSceneType {
  SULayerVisibilityNewSceneType_LayerDefault = 0x0000,
  SULayerVisibilityNewSceneType_Visible = 0x0010,
  SULayerVisibilityNewSceneType_Hidden = 0x0020
};

/**
@brief Converts from an \ref SULayerRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] layer  The given layer reference.
@related SULayerRef
@return
- The converted \ref SUEntityRef if layer is a valid layer
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SULayerToEntity(SULayerRef layer);

/**
@brief Converts from an \ref SUEntityRef to an \ref SULayerRef.
       This is essentially a downcast operation so the given
       SUEntityRef must be convertible to an \ref SULayerRef.
@param[in] entity The given entity reference.
@related SULayerRef
@return
- The converted \ref SULayerRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SULayerRef SULayerFromEntity(SUEntityRef entity);

/**
@brief Creates a new layer object.

Layers associated with a SketchUp model must not be explicitly deallocated.
Layers that are not associated with a SketchUp model must be deallocated with
\ref SULayerRelease().
@param[out] layer The layer object created.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer is NULL
*/
SU_RESULT SULayerCreate(SULayerRef* layer);

/**
@brief Deallocates a layer object.

The layer object to be deallocated must not be associated with a SketchUp model.
@param[in] layer The layer object.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer points to an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if layer is NULL
*/
SU_RESULT SULayerRelease(SULayerRef* layer);

/**
@brief Retrieves the name of a layer object.
@param[in]  layer The layer object.
@param[out] name  The name retrieved.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SULayerGetName(SULayerRef layer, SUStringRef* name);

/**
@brief Retrieves the display name of a layer object.
@since SketchUp 2020.0, API version 8.0

The display name is the name you see in the product.  In SketchUp 2020 the
display name for the default layer changed from "Layer0" to "Untagged".
For all other layers the name and display name are identical.

@param[in]  layer The layer object.
@param[out] name  The display name retrieved.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SULayerGetDisplayName(SULayerRef layer, SUStringRef* name);

/**
@brief Assigns the name of a layer object.
@param[in] layer The layer object.
@param[in] name  The new name of the layer object. Assumed to be UTF-8 encoded.
@related SULayerRef
@return
- \ref SU_ERROR_NONE if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SULayerSetName(SULayerRef layer, const char* name);

/**
@brief Retrieves the material object associated with a layer object.

The retrieved material object must not be deallocated by the caller since it is
owned by the layer object.
@param[in]  layer    The layer object.
@param[out] material The material object retrieved.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if material is NULL
*/
SU_RESULT SULayerGetMaterial(SULayerRef layer, SUMaterialRef* material);

/**
@brief Retrieves the boolean flag indicating whether a layer object is visible.
@param[in]  layer   The layer object.
@param[out] visible The visibility flag retrieved.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if visibility is NULL
*/
SU_RESULT SULayerGetVisibility(SULayerRef layer, bool* visible);

/**
@brief Sets the boolean flag indicating whether a layer object is visible.
@param[in] layer   The layer object.
@param[in] visible The visibility flag to set.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object.
*/
SU_RESULT SULayerSetVisibility(SULayerRef layer, bool visible);

/**
@brief Get the line style of a layer.
@since SketchUp 2019, API 7.0
@param[in]  layer      The layer object.
@param[out] line_style The line style reference.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if line_style is NULL
- \ref SU_ERROR_NO_DATA if the layer has no custom line style.
*/
SU_RESULT SULayerGetLineStyle(SULayerRef layer, SULineStyleRef* line_style);

/**
@brief Set the line style of a layer.
@since SketchUp 2019, API 7.0
@param[in] layer      The layer object.
@param[in] line_style The line style to set.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer or line_style is not a valid object.
*/
SU_RESULT SULayerSetLineStyle(SULayerRef layer, SULineStyleRef line_style);

/**
@brief Clear the line style of a layer.
@since SketchUp 2019, API 7.0
@param[in] layer The layer object.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer is not a valid object.
 */
SU_RESULT SULayerClearLineStyle(SULayerRef layer);

/**
@brief Get the scene behavior on the layer.
@since SketchUp 2020.0, API 8.0
@param[in]    layer          The layer object.
@param[out]   default_type   The retrieved default scene behavior.
@param[out]   new_scene_type The retrieved behavior for new scenes.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the layer is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if default_type or new_scene_type is NULL
*/
SU_RESULT SULayerGetSceneBehavior(
    SULayerRef layer, enum SULayerVisibilityDefaultType* default_type,
    enum SULayerVisibilityNewSceneType* new_scene_type);

/**
@brief Set the scene behavior on the layer.
@since SketchUp 2020.0, API 8.0
@param[in]  layer           The layer object.
@param[in]  default_type    The default scene behavior.
@param[in]  new_scene_type  The behavior to set for new scenes.
@related SULayerRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if the layer is not a valid object
*/
SU_RESULT SULayerSetSceneBehavior(
    SULayerRef layer, enum SULayerVisibilityDefaultType default_type,
    enum SULayerVisibilityNewSceneType new_scene_type);

/**
 @brief Gets the \ref SULayerFolderRef object that contains the given layer.
 @since SketchUp 2020.2, API 8.2
 @param[in]  layer            The layer object.
 @param[out]  layer_folder    The retrieved layer folder object.
 @related SULayerRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer is not a valid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p layer_folder is NULL
 - \ref SU_ERROR_NO_DATA if \p layer is not contained within a layer folder
*/
SU_RESULT SULayerGetParentLayerFolder(SULayerRef layer, SULayerFolderRef* layer_folder);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_LAYER_H_
