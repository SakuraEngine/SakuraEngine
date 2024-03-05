// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_PAGE_H_
#define LAYOUT_MODEL_PAGE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@enum LOPageEntityIteratorFlags
@brief Defines the options that are available when creating a \ref
       LOEntityIteratorRef for iterating the entities on a page.
*/
typedef enum {
  LOPageEntityIteratorFlags_None = 0x0,  ///< Default iterator behavior. Does not skip any entities.
  LOPageEntityIteratorFlags_SkipHidden = 0x1,         ///< Skip entities on hidden layers
  LOPageEntityIteratorFlags_SkipLocked = 0x2,         ///< Skip entities on locked layers
  LOPageEntityIteratorFlags_SkipHiddenOrLocked = 0x3  ///< Skip entities on hidden or locked layers
} LOPageEntityIteratorFlags;

/**
@struct LOPageRef
@brief References a single page in a document.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the name of a page.
@param[in]  page The page object.
@param[out] name The name of the page.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not refer to a valid object
*/
LO_RESULT LOPageGetName(LOPageRef page, SUStringRef* name);

/**
@brief Sets the name of a page.
@param[in] page The page object.
@param[in] name The new name of the page.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_UNSUPPORTED if name is an empty string
*/
LO_RESULT LOPageSetName(LOPageRef page, const char* name);

/**
@brief Gets whether or not a page is included in presentations.
@param[in]  page            The page object.
@param[out] in_presentation Whether the page will be in a presentation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if in_presentation is NULL
*/
LO_RESULT LOPageGetInPresentation(LOPageRef page, bool* in_presentation);

/**
@brief Sets whether or not a page is included in presentations.
@param[in] page            The page object.
@param[in] in_presentation Whether the page should be in a presentation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
*/
LO_RESULT LOPageSetInPresentation(LOPageRef page, bool in_presentation);

/**
@brief Gets whether or not a layer is visible on a page. Layers in LayOut exist
       on every page in a document. Each page specifies the visibility of each
       layer for that page.
@param[in]  page             The page object.
@param[in]  layer_definition The layer definition object.
@param[out] visible          Whether or not the layer is visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if visible is NULL
- \ref SU_ERROR_GENERIC if page and layer_definition don't belong to the same
  document
*/
LO_RESULT LOPageGetLayerVisible(LOPageRef page, LOLayerRef layer_definition, bool* visible);

/**
@brief Sets whether or not a layer is visible on a page. Layers in LayOut
       exist on every page in a document. Each page specifies the visibility of
       each layer for that page.
@param[in] page             The page object.
@param[in] layer_definition The layer definition object.
@param[in] visible          Whether or not the layer should be visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_GENERIC if page and layer_definition don't belong to the same
  document
- \ref SU_ERROR_GENERIC if the layer could not be hidden because it would
  break the rule that there must be at least one unlocked, visible layer on
  each page
*/
LO_RESULT LOPageSetLayerVisible(LOPageRef page, LOLayerRef layer_definition, bool visible);

/**
@brief Gets references to all the layer instances for a page. The layer
       instances are in the same order that the layer definitions exist in the
       document. Use \ref LODocumentGetNumberOfLayers to determine how many
       layer instances will be returned by this function.
@param[in]  page            The page object.
@param[in]  array_size      The size of the array to copy into.
@param[out] layer_instances The layer instances.
@param[out] number_copied   The number of layer instances copied.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_instances is NULL
- \ref SU_ERROR_OVERWRITE_VALID if one or more of the objects in the
  layer_instances array are not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if number_copied is NULL
*/
LO_RESULT LOPageGetLayerInstances(
    LOPageRef page, size_t array_size, LOLayerInstanceRef layer_instances[], size_t* number_copied);

/**
@brief Gets a reference to the layer instance at the given index for this
       page. The layer instances are in the same order that the layer
       definitions exist in the document. Use \ref LODocumentGetNumberOfLayers
       to determine how many layer definitions exist in the document.
@param[in]  page           The page object.
@param[in]  index          The index of the layer instance to get.
@param[out] layer_instance The layer instance object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_instance is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_instance already refers to a valid object
*/
LO_RESULT LOPageGetLayerInstanceAtIndex(
    LOPageRef page, size_t index, LOLayerInstanceRef* layer_instance);

/**
@brief Gets the index of this page in the document.
@param[in]  page  The page object.
@param[out] index The index of the page in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if index is NULL
- \ref SU_ERROR_GENERIC if page doesn't belong to a document
*/
LO_RESULT LOPageGetPageIndex(LOPageRef page, size_t* index);

/**
@brief Gets the number of entities at the top of the group hierarchy, on
       non-shared layers for the given page. This count will include
       \ref LOGroupRef entities so the group hierarchy can be traversed.
@param[in]  page                   The page object.
@param[out] num_nonshared_entities The number of non-shared entities.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_nonshared_entities is NULL
*/
LO_RESULT LOPageGetNumberOfNonSharedEntities(LOPageRef page, size_t* num_nonshared_entities);

/**
@brief Gets the non-shared entity at the top of the group hierarchy at the
       specified index for the given page.
@param[in]  page   The page object.
@param[in]  index  The 0-based entity index for the desired non-shared entity.
@param[out] entity The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity already refers to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is greater than or equal to the number of
  non-shared entities returned by LOPageGetNumberOfNonSharedEntities
*/
LO_RESULT LOPageGetNonSharedEntityAtIndex(LOPageRef page, size_t index, LOEntityRef* entity);

/**
@brief Populates a \ref LOEntityListRef with the entities at the top of the
       group hierarchy, on non-shared layers for the given page. This will
       include \ref LOGroupRef entities so the group hierarchy can be
       traversed.
@param[in]  page        The page object.
@param[out] entity_list The entity list to populate.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOPageGetNonSharedEntities(LOPageRef page, LOEntityListRef entity_list);

/**
@brief Creates a new entity iterator for the given page. See \ref
       LOPageEntityIteratorFlags for valid options for flags. This iterator
       will visit all entities on the page in exactly the same order they are
       drawn, including entities on shared layers. Entity group hierarchy is
       ignored with this iterator, so groups are skipped. This is the
       recommended method for entity iteration when drawing or exporting.
@param[in]  page               The page object.
@param[in]  flags              Flags indicating what entities to include.
@param[out] entity_iterator    The entity iterator object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity_iterator is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity_iterator already refers to a valid object
*/
LO_RESULT LOPageCreateEntityIterator(
    LOPageRef page, LOPageEntityIteratorFlags flags, LOEntityIteratorRef* entity_iterator);

/**
@brief Creates a new reverse entity iterator for the given page. See \ref
       LOPageEntityIteratorFlags for valid options for flags. This iterator
       will visit all entities on the page in the reverse order they are drawn,
       including entities on shared layers. Entity group hierarchy is ignored
       with this iterator, so groups are skipped. This is the recommended
       method for entity iteration when picking.
@param[in]  page               The page object.
@param[in]  flags              Flags indicating what entities to include.
@param[out] entity_iterator    The entity iterator object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity_iterator is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity_iterator already refers to a valid object
*/
LO_RESULT LOPageCreateReverseEntityIterator(
    LOPageRef page, LOPageEntityIteratorFlags flags, LOEntityIteratorRef* entity_iterator);

/**
@brief Returns the document that this page belongs to.
@since LayOut 2018, API 3.0
@param[in]  page     The page object.
@param[out] document The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
*/
LO_RESULT LOPageGetDocument(LOPageRef page, LODocumentRef* document);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_PAGE_H_
