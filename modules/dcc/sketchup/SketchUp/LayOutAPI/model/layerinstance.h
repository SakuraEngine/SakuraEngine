// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_LAYER_INSTANCE_H_
#define LAYOUT_MODEL_LAYER_INSTANCE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOLayerInstanceRef
@brief References a layer instance. A layer instance provides access to the
       entities that are drawn on a given layer, as well as the draw order of
       those entities. Groups are not included in the list of entities
       associated with a layer instance, since the group hierarchy has no
       effect on entity draw order. Each page has one layer instance for each
       layer in the document. Non-shared layer instances are unique per page,
       while shared layer instances are shared across all pages of the
       document.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the number of entities associated with a layer instance.
@param[in]  layer_instance     The layer instance object.
@param[out] number_of_entities The number of entities associated with the layer
            instance.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if number_of_entities is NULL
*/
LO_RESULT LOLayerInstanceGetNumberOfEntities(
    LOLayerInstanceRef layer_instance, size_t* number_of_entities);

/**
@brief Gets the entity at the given index of a layer instance's draw order.
@param[in]  layer_instance The layer instance object.
@param[in]  index          The index of the entity to get.
@param[out] entity         The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index out of range
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity already refers to a valid object
*/
LO_RESULT LOLayerInstanceGetEntityAtIndex(
    LOLayerInstanceRef layer_instance, size_t index, LOEntityRef* entity);

/**
@brief Gets the index of the given entity on the layer instance.
@since LayOut 2018, API 3.0
@param[in]  layer_instance The layer instance object.
@param[in]  entity         The entity object.
@param[out] index          The index of the entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if entity is not on layer_instance
- \ref SU_ERROR_NULL_POINTER_OUTPUT if index is NULL
*/
LO_RESULT LOLayerInstanceGetIndexOfEntity(
    LOLayerInstanceRef layer_instance, LOEntityRef entity, size_t* index);

/**
@brief Populates a \ref LOEntityListRef with all the entities associated with a
       layer instance, ordered by their draw order.
@param[in]  layer_instance The layer instance object.
@param[out] entity_list    The entity list to populate.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOLayerInstanceGetEntities(
    LOLayerInstanceRef layer_instance, LOEntityListRef entity_list);

/**
@brief Moves the entity to the specified index within a layer instance's draw
       order. The entity must already belong to the layer instance that is
       passed in.
@param[in] layer_instance The layer instance object.
@param[in] entity         The entity object.
@param[in] index          The index to move the entity to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity does not belong to the given layer
  instance
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range
*/
LO_RESULT LOLayerInstanceReorderEntity(
    LOLayerInstanceRef layer_instance, LOEntityRef entity, size_t index);

/**
@brief Gets the layer definition that a layer instance is associated with.
@param[in]  layer_instance   The layer instance object.
@param[out] layer_definition The layer definition object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_instance does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_definition is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_definition already refers to a valid object
*/
LO_RESULT LOLayerInstanceGetLayerDefinition(
    LOLayerInstanceRef layer_instance, LOLayerRef* layer_definition);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_LAYER_INSTANCE_H_