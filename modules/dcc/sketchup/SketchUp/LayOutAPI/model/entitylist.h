// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_ENTITY_LIST_H_
#define LAYOUT_MODEL_ENTITY_LIST_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOEntityListRef
@brief References an ordered, indexable list of \ref LOEntityRef objects.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new entity list object.
@param[out] entity_list The entity list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity_list is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity_list already refers to a valid
  object
*/
LO_RESULT LOEntityListCreate(LOEntityListRef* entity_list);

/**
@brief Releases an entity list object. *entity_list will be set to invalid by
       this function.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if entity_list is NULL
- \ref SU_ERROR_INVALID_INPUT if *entity_list does not refer to a valid object
*/
LO_RESULT LOEntityListRelease(LOEntityListRef* entity_list);

/**
@brief Adds an entity to an entity list object.
@param[in] entity_list The entity list object.
@param[in] entity      The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
*/
LO_RESULT LOEntityListAddEntity(LOEntityListRef entity_list, LOEntityRef entity);

/**
@brief Gets the number of entities in an entity list object.
@param[in]  entity_list  The entity list object.
@param[out] num_entities The number of entities in this list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_entities is NULL
*/
LO_RESULT LOEntityListGetNumberOfEntities(LOEntityListRef entity_list, size_t* num_entities);

/**
@brief Gets the entity at the specified index in an entity list object.
@param[in]  entity_list The entity list object.
@param[in]  index       The index of the entity to get.
@param[out] entity      The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range for this list
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity already refers to a valid object
*/
LO_RESULT LOEntityListGetEntityAtIndex(
    LOEntityListRef entity_list, size_t index, LOEntityRef* entity);

/**
@brief Moves a list of entities to the given layer.

If the layer is non-shared and any entity is on a shared layer, pages must be valid and populated
with the pages to move the entities to. In all other cases, pages may be an invalid object. The
entities must belong to the same document as the layer and pages.

@bug In LayOut versions prior to LayOut 2024.0 this method would fail to move entities from
  non-shared layers.

@param[in] entity_list The entity list object.
@param[in] layer       The layer definition object.
@param[in] pages       The page list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p entity_list does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if \p entity_list is empty
- \ref SU_ERROR_INVALID_INPUT if \p entity_list contains the same entity reference more
  than once
- \ref SU_ERROR_INVALID_INPUT if \p layer does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if \p pages does not refer to a valid object and
  \p layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if \p pages is empty and \p layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if \p pages contains the same page reference more
  than once and \p layer is non-shared
- \ref SU_ERROR_GENERIC if any entity, \p layer, and \p pages are not all in the
  same document
- \ref SU_ERROR_LAYER_LOCKED if \p layer is locked or if any entity is currently
  on a locked layer
*/
LO_RESULT LOEntityListMoveToLayer(
    LOEntityListRef entity_list, LOLayerRef layer, LOPageListRef pages);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_ENTITY_LIST_H_
