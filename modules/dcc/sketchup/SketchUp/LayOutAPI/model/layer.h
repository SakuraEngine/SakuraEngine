// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_LAYER_H_
#define LAYOUT_MODEL_LAYER_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOLayerRef
@brief References a layer definition. A layer definition specifies the
       document-wide information about a layer. To access the entities on a
       layer for a given page, use \ref LOLayerInstanceRef.
*/

/**
@enum LOShareLayerAction
@brief Defines the different ways to manage entities when sharing a layer.
*/
typedef enum {
  LOShareLayerAction_Clear,          ///< Delete all entities on the layer
  LOShareLayerAction_KeepOnePage,    ///< Keep the entities on the specified page
  LOShareLayerAction_MergeAllPages,  ///< Merge the entities from all pages

  LONumShareLayerActions
} LOShareLayerAction;

/**
@enum LOUnshareLayerAction
@brief Defines the different ways to manage entities when making a layer
       non-shared.
*/
typedef enum {
  LOUnshareLayerAction_Clear,           ///< Delete all entities on the layer
  LOUnshareLayerAction_CopyToOnePage,   ///< Copy entities to the specified page
  LOUnshareLayerAction_CopyToAllPages,  ///< Copy entities to all pages

  LONumUnshareLayerActions
} LOUnshareLayerAction;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the name of a layer.
@param[in]  layer_definition The layer definition object.
@param[out] name             The name of the layer.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not refer to a valid object
*/
LO_RESULT LOLayerGetName(LOLayerRef layer_definition, SUStringRef* name);

/**
@brief Sets the name of a layer.
@param[in] layer_definition The layer definition object.
@param[in] name             The new name for the layer.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_UNSUPPORTED if name is an empty string
*/
LO_RESULT LOLayerSetName(LOLayerRef layer_definition, const char* name);

/**
@brief Gets whether or not a layer is locked.
@param[in]  layer_definition The layer definition object.
@param[out] is_locked        Whether the layer is locked or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_locked is NULL
*/
LO_RESULT LOLayerGetLocked(LOLayerRef layer_definition, bool* is_locked);

/**
@brief Sets whether or not a layer is locked. When setting a layer to locked,
       there must be at least one other unlocked and visible layer on every
       page. If this is not the case, then the next layer will be automatically
       unlocked and made visible on all pages as necessary to proceed with the
       operation.
@param[in] layer_definition The layer definition object.
@param[in] is_locked        Whether the layer should be locked or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_GENERIC if the layer could not be locked because it would
  break the rule that there must be at least one unlocked, visible layer on
  each page
*/
LO_RESULT LOLayerSetLocked(LOLayerRef layer_definition, bool is_locked);

/**
@brief Gets whether or not a layer is a shared layer.
@param[in] layer_definition The layer definition object.
@param[in] is_shared        Whether the layer is shared or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_shared is NULL
*/
LO_RESULT LOLayerGetShared(LOLayerRef layer_definition, bool* is_shared);

/**
@brief Shares a layer. If action is \ref LOShareLayerAction_Clear or
       \ref LOShareLayerAction_MergeAllPages, then page may be an invalid
       object.
@param[in] layer_definition The layer definition object.
@param[in] page             The the page to use when action is \ref
                            LOShareLayerAction_KeepOnePage.
@param[in] action           The action to apply to existing entities on the
                            layer that is being shared.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_LAYER_LOCKED if layer_definition is a locked layer
- \ref SU_ERROR_GENERIC if page does not refer to a valid object and the
  desired action requires a page
- \ref SU_ERROR_GENERIC if an error occured attempting to share the layer
- \ref SU_ERROR_OUT_OF_RANGE if action is not a valid value
*/
LO_RESULT LOLayerSetShared(LOLayerRef layer_definition, LOPageRef page, LOShareLayerAction action);

/**
@brief Unshares a layer. If action is \ref LOUnshareLayerAction_Clear or
       \ref LOUnshareLayerAction_CopyToAllPages, then page may be an invalid
       object.
@param[in] layer_definition The layer definition object.
@param[in] page             The the page to use when action is
                            \ref LOUnshareLayerAction_CopyToOnePage.
@param[in] action           The action to apply to existing entities on the
                            layer that is being unshared.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_LAYER_LOCKED if layer_definition is a locked layer
- \ref SU_ERROR_GENERIC if page does not refer to a valid object and the
  desired action requires a page
- \ref SU_ERROR_GENERIC if an error occured attempting to unshare the layer
- \ref SU_ERROR_OUT_OF_RANGE if action is not a valid value
*/
LO_RESULT LOLayerSetNonShared(
    LOLayerRef layer_definition, LOPageRef page, LOUnshareLayerAction action);

/**
@brief Gets the layer instance for the given layer definition on a given page.
       If layer_definition specifies a shared layer, page may be an invalid
       object.
@param[in]  layer_definition The layer definition object.
@param[in]  page             The page object.
@param[out] layer_instance   The layer instance object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object and
  layer_definition is not a shared layer
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_instance is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_instance already refers to a valid object
- \ref SU_ERROR_GENERIC if an error occurred attempting to get the layer
  instance
*/
LO_RESULT LOLayerGetLayerInstance(
    LOLayerRef layer_definition, LOPageRef page, LOLayerInstanceRef* layer_instance);

/**
@brief Returns the index of this layer in the document.
@param[in]  layer_definition The layer definition object.
@param[out] index            The index of the layer in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if index is NULL
*/
LO_RESULT LOLayerGetLayerIndex(LOLayerRef layer_definition, size_t* index);

/**
@brief Returns the document that this layer belongs to.
@since LayOut 2018, API 3.0
@param[in]  layer_definition The layer definition object.
@param[out] document         The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
*/
LO_RESULT LOLayerGetDocument(LOLayerRef layer_definition, LODocumentRef* document);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_LAYER_H_
