// Copyright 2020 Trimble, Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SULayerFolderRef.
 */
#ifndef SKETCHUP_MODEL_LAYER_FOLDER_H_
#define SKETCHUP_MODEL_LAYER_FOLDER_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SULayerFolderRef
@brief References a Tag Folder object.
@note 'Layer' is a legacy term that is being used for consistency within the API.
@since SketchUp 2021.0, API 9.0
*/

/**
@brief Converts from an \ref SULayerFolderRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder  The given layer folder reference.
@related SULayerFolderRef
@return
- The converted \ref SUEntityRef if \p layer_folder is a valid layer folder
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SULayerFolderToEntity(SULayerFolderRef layer_folder);

/**
@brief Converts from an \ref SUEntityRef to an \ref SULayerFolderRef.
       This is essentially a downcast operation so the given
       SUEntityRef must be convertible to an \ref SULayerFolderRef.
@since SketchUp 2021.0, API 9.0
@param[in] entity The given entity reference.
@related SULayerFolderRef
@return
- The converted \ref SULayerFolderRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SULayerFolderRef SULayerFolderFromEntity(SUEntityRef entity);

/**
@brief Creates a new layer folder object with the given name.

Layer Folders associated with a SketchUp model must not be explicitly
deallocated.
Layer folders that are not associated with a SketchUp model must be deallocated
with SULayerFolderRelease().

@note Layer folder names do not need to be unique, but they can not be empty.
@since SketchUp 2021.0, API 9.0
@param[out] layer_folder The layer folder object created.
@param[in] name The desired layer folder name. Assumed to be UTF-8 encoded.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p layer_folder is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if \p name is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if \p name is an empty string
*/
SU_RESULT SULayerFolderCreate(SULayerFolderRef* layer_folder, const char* name);

/**
@brief Deallocates a layer folder object.

The layer folder object to be deallocated must not be associated with a
SketchUp model.
@note To release a \ref SULayerFolderRef that is owned by a model you should
use SUModelRemoveLayerFolders().
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder The layer folder object.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder points to an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p layer_folder is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if \p layer_folder is contained within a model.
*/
SU_RESULT SULayerFolderRelease(SULayerFolderRef* layer_folder);

/**
@brief Retrieves the name of a layer folder object.
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder The layer folder object.
@param[out] name  The name retrieved.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if \p name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SULayerFolderGetName(SULayerFolderRef layer_folder, SUStringRef* name);

/**
@brief Assigns the name of a layer folder object.
@note Layer folder names do not need to be unique, but they can not be empty.
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder The layer folder object.
@param[in] name  The new name of the layer folder object. Assumed to be UTF-8
                 encoded.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE if \p layer_folder is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p name is NULL
- \ref SU_ERROR_INVALID_ARGUMENT if \p name is an empty string
*/
SU_RESULT SULayerFolderSetName(SULayerFolderRef layer_folder, const char* name);

/**
@brief Retrieves the boolean flag indicating whether a layer folder object is
       visible.
@since SketchUp 2021.0, API 9.0
@param[in]  layer_folder   The layer folder object.
@param[out] visible The visibility flag retrieved.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p visible is NULL
*/
SU_RESULT SULayerFolderGetVisibility(SULayerFolderRef layer_folder, bool* visible);

/**
@brief Sets the boolean flag indicating whether a layer folder object is
       visible.
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder   The layer folder object.
@param[in] visible The visibility flag to set.
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object.
*/
SU_RESULT SULayerFolderSetVisibility(SULayerFolderRef layer_folder, bool visible);

/**
@brief Retrieves the boolean flag indicating whether a layer folder object is
       visible by default on new scenes.
@since SketchUp 2021.0, API 9.0
@param[in]  layer_folder  The layer folder object.
@param[out] visible  The visibility flag retrieved.
@see SUSceneLayerFolders()
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p visible is NULL
*/
SU_RESULT SULayerGroupGetVisibleOnNewScenes(SULayerFolderRef layer_folder, bool* visible);

/**
@brief Sets the boolean flag indicating whether a layer folder object is
       visible by default on new scenes.
@since SketchUp 2021.0, API 9.0
@param[in] layer_folder  The layer folder object.
@param[in] visible The  visibility flag to set.
@see SUSceneAddLayerFolder()
@see SUSceneRemoveLayerFolder()
@related SULayerFolderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object.
*/
SU_RESULT SULayerGroupSetVisibleOnNewScenes(SULayerFolderRef layer_folder, bool visible);

/**
 @brief Adds a layer to the given layer folder.
 @since SketchUp 2021.0, API 9.0
 @param[in] layer_folder   The layer folder object.
 @param[in] layer  The layer to add.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder or \p layer is not a valid
   object.
 - \ref SU_ERROR_INVALID_ARGUMENT if \p layer can not be added to, or is
   already a member of, \p layer_folder.
*/
SU_RESULT SULayerFolderAddLayer(SULayerFolderRef layer_folder, SULayerRef layer);

/**
 @brief Removes a layer from the given layer folder.
 @since SketchUp 2021.0, API 9.0
 @param[in] layer_folder   The layer folder object.
 @param[in] layer  The layer to remove.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder or \p layer is not a valid object.
 - \ref SU_ERROR_INVALID_ARGUMENT if \p layer is not in \p layer_folder.
*/
SU_RESULT SULayerFolderRemoveLayer(SULayerFolderRef layer_folder, SULayerRef layer);

/**
 @brief Gets the number of layers that layer_folder contains.
 @since SketchUp 2021.0, API 9.0
 @param[in] layer_folder   The layer folder object.
 @param[out] count  The number of layers.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object.
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p count is NULL
*/
SU_RESULT SULayerFolderGetNumLayers(SULayerFolderRef layer_folder, size_t* count);

/**
 @brief Gets the layers that are in the layer_folder
 @since SketchUp 2021.0, API 9.0
 @param[in] layer_folder   The layer folder object.
 @param[in] len  The number of elements in the layers array.
 @param[out] layers  The layers that are in the layer folder.
 @param[out] count  The number of layers read into the layers array.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object.
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p layers or \p count is NULL.
*/
SU_RESULT SULayerFolderGetLayers(
    SULayerFolderRef layer_folder, size_t len, SULayerRef* layers, size_t* count);

/**
 @brief Adds a \ref SULayerFolderRef object to the given layer folder.
 @since SketchUp 2021.0, API 9.0
 @param[in]  layer_folder   The parent layer folder.
 @param[in]  add_folder     The layer folder to add.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder or \p add_folder is an invalid
 object
 - \ref SU_ERROR_INVALID_ARGUMENT if \p layer_folder is not part of the model or
 \p add_folder fails to be added or is already contained within \p layer_folder.
 */
SU_RESULT SULayerFolderAddLayerFolder(SULayerFolderRef layer_folder, SULayerFolderRef add_folder);

/**
 @brief Gets the number of \ref SULayerFolderRef objects that are direct
 children of the given layer folder object.
 @since SketchUp 2021.0, API 9.0
 @param[in]   layer_folder  The layer folder object.
 @param[out]  count         The number of layer folder objects that are direct
 children.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p model is an invalid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p count is NULL
*/
SU_RESULT SULayerFolderGetNumLayerFolders(SULayerFolderRef layer_folder, size_t* count);

/**
 @brief Gets the \ref SULayerFolderRef objects that are direct children of the
 given layer folder object.
 @since SketchUp 2021.0, API 9.0
 @param[in]  layer_folder    The layer folder object.
 @param[in]  len             The number of elements in \p layer_folders.
 @param[out]  layer_folders  An array of layer folder objects.
 @param[out]  count          The number of elements written into \p layer_folders.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p model is an invalid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p layer_folders or \p count is NULL
*/
SU_RESULT SULayerFolderGetLayerFolders(
    SULayerFolderRef layer_folder, size_t len, SULayerFolderRef* layer_folders, size_t* count);

/**
 @brief Gets the \ref SULayerFolderRef object that contains the given layer
 folder.
 @since SketchUp 2021.0, API 9.0
 @param[in]   layer_folder    The layer folder object.
 @param[out]  parent          The retrieved parent layer folder object.
 @related SULayerFolderRef
 @return
 - \ref SU_ERROR_NONE on success
 - \ref SU_ERROR_INVALID_INPUT if \p layer_folder is not a valid object
 - \ref SU_ERROR_NULL_POINTER_OUTPUT if \p layer_folder is NULL
 - \ref SU_ERROR_NO_DATA if \p layer_folder is not contained within a layer
 folder
*/
SU_RESULT SULayerFolderGetParentLayerFolder(
    SULayerFolderRef layer_folder, SULayerFolderRef* parent);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_LAYER_FOLDER_H_
