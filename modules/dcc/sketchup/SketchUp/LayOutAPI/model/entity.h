// Copyright 2015-2022 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_ENTITY_H_
#define LAYOUT_MODEL_ENTITY_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@enum LOEntityType
@brief Defines the different types of entities that may exist in a document.
*/
typedef enum {
  LOEntityType_FormattedText = 0,  ///< The entity is a LOFormattedTextRef.
  LOEntityType_Group,              ///< The entity is a LOGroupRef.
  LOEntityType_Image,              ///< The entity is a LOImageRef.
  LOEntityType_LinearDimension,    ///< The entity is a LOLinearDimensionRef.
  LOEntityType_Path,               ///< The entity is a LOPathRef.
  LOEntityType_Rectangle,          ///< The entity is a LORectangleRef.
  LOEntityType_SketchUpModel,      ///< The entity is a LOSketchUpModelRef.
  LOEntityType_Ellipse,            ///< The entity is a LOEllipseRef.
  LOEntityType_Label,              ///< The entity is a LOLabelRef.
  LOEntityType_Table,              ///< The entity is a LOTableRef.
  LOEntityType_AngularDimension,   ///< The entity is a LOAngularDimensionRef.
  LOEntityType_ReferenceEntity,    ///< The entity is a LOReferenceEntityRef.
  LONumEntityTypes
} LOEntityType;

/**
@struct LOEntityRef
@brief An entity is an object shown on a page of a LayOut document.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the bounds of an entity in the x and y axis of the page.
@return
@param[in]  entity The entity object.
@param[out] bounds The bounds of the entity.
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bounds is NULL
*/
LO_RESULT LOEntityGetAxisAlignedBounds(LOEntityRef entity, LOAxisAlignedRect2D* bounds);

/**
@brief Gets a rectangle that bounds an entity. The rectangle returned by this
       function may not be aligned with the page axis if the entity has an
       explicit transformation.
@param[in]  entity The entity object.
@param[out] bounds The oriented bounds of the entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bounds is NULL
*/
LO_RESULT LOEntityGetOrientedBounds(LOEntityRef entity, LOOrientedRect2D* bounds);

/**
@brief Gets whether or not an entity has an explicit transformation. The
       following types of entities will always have an explicit transformation:
       \ref LOFormattedTextRef, \ref LOImageRef, \ref LORectangleRef,
       \ref LOSketchUpModelRef and \ref LOEllipseRef.
@param[in]  entity        The entity object.
@param[out] has_transform Whether the entity has an explicit transformation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if has_transform is NULL
*/
LO_RESULT LOEntityHasExplicitTransform(LOEntityRef entity, bool* has_transform);

/**
@brief Gets the explicit transformation for an entity. This function is only
       supported for entities that have an explicit transformation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NO_DATA if entity does not have an explicit transformation
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform_matrix is NULL
*/
LO_RESULT LOEntityGetExplicitTransform(LOEntityRef entity, LOTransformMatrix2D* transform_matrix);

/**
@brief Applies a transformation to an entity. This works for all types of
       entities, regardless of whether or not they have an explicit
       transformation. It even works on groups, in which case the
       transformation is applied to everything inside the group.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if entity is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if entity is locked
- \ref SU_ERROR_NULL_POINTER_INPUT if transform_matrix is NULL
*/
LO_RESULT LOEntityApplyTransform(LOEntityRef entity, const LOTransformMatrix2D* transform_matrix);

/**
@brief Gets the untransformed bounds of an entity. This is the bounds of the
       entity before its explicit transformation is applied. This function is
       only supported for entities that have an explicit transformation.
@param[in]  entity The entity object.
@param[out] bounds The untransformed bounds.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NO_DATA if entity does not have an explicit transformation
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bounds is NULL
*/
LO_RESULT LOEntityGetUntransformedBounds(LOEntityRef entity, LOAxisAlignedRect2D* bounds);

/**
@brief Sets the untransformed bounds of an entity. This is the bounds of the
       entity before its explicit transformation is applied. This function is
       only supported for entities that have an explicit transformation.
@param[in] entity The entity object.
@param[in] bounds The untransformed bounds.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_GENERIC if entity does not have an explicit transformation
- \ref SU_ERROR_LAYER_LOCKED if entity is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if entity is locked
- \ref SU_ERROR_NULL_POINTER_OUTPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds is zero sized
*/
LO_RESULT LOEntitySetUntransformedBounds(LOEntityRef entity, const LOAxisAlignedRect2D* bounds);

/**
@brief Gets the entity type of an entity. This can be used to determine which
       FromEntity function to use for casting the LOEntityRef object to its
       specific entity object type.
@param[in]  entity      The entity object.
@param[out] entity_type The entity type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity_type is NULL
*/
LO_RESULT LOEntityGetEntityType(LOEntityRef entity, LOEntityType* entity_type);

/**
@brief Gets the document that an entity belongs to.
@param[in]  entity   The entity object.
@param[out] document The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *document already refers to a valid object
- \ref SU_ERROR_NO_DATA if this entity is not in a document
*/
LO_RESULT LOEntityGetDocument(LOEntityRef entity, LODocumentRef* document);

/**
@brief Gets the layer instance that an entity is associated with. Note that
       groups are never associated with a layer instance.
@param[in]  entity         The entity object.
@param[out] layer_instance The layer instance object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NO_DATA if this entity is not associated with a layer or does
  not belong to a document
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_instance is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_instance already refers to a valid
object
*/
LO_RESULT LOEntityGetLayerInstance(LOEntityRef entity, LOLayerInstanceRef* layer_instance);

/**
@brief Moves an entity to the given layer. If the layer is non-shared and the
       entity is is currently on a shared layer, pages must be valid and
       populated with the pages to move the entity to. In all other cases,
       pages may be an invalid object. The entity must belong to the same
       document as the the layer and the pages.
@param[in] entity The entity object.
@param[in] layer  The layer definition object.
@param[in] pages  The page list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if pages does not refer to a valid object and
  layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if pages is empty and layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if pages contains the same page reference more
  than once and layer is non-shared
- \ref SU_ERROR_GENERIC if entity, layer, and pages are not all in the same
  document
- \ref SU_ERROR_LAYER_LOCKED if layer is locked or if entity is currently on a
  locked layer
- \ref SU_ERROR_ENTITY_LOCKED if entity is locked
*/
LO_RESULT LOEntityMoveToLayer(LOEntityRef entity, LOLayerRef layer, LOPageListRef pages);

/**
@brief Moves a list of entities to the given layer. If the layer is non-shared
       and any entity is on a shared layer, pages must be valid and populated
       with the pages to move the entities to. In all other cases, pages may be
       an invalid object. The entities must belong to the same document as the
       layer and pages.
@param[in] entities The entity object.
@param[in] layer    The layer definition object.
@param[in] pages    The page list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entities does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entities is empty
- \ref SU_ERROR_INVALID_INPUT if entities contains the same entity reference more
  than once
- \ref SU_ERROR_INVALID_INPUT if layer does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if pages does not refer to a valid object and
  layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if pages is empty and layer is non-shared
- \ref SU_ERROR_INVALID_INPUT if pages contains the same page reference more
  than once and layer is non-shared
- \ref SU_ERROR_GENERIC if the entities, layer, and pages are not all in the
  same document
- \ref SU_ERROR_LAYER_LOCKED if layer is locked or if any entity is currently
  on a locked layer
*/
LO_RESULT LOEntityListMoveToLayer(LOEntityListRef entities, LOLayerRef layer, LOPageListRef pages);

/**
@brief Gets the page that an entity belongs to. This function will only
       succeed if the entity is on a non-shared layer.
@param[in]  entity The entity object.
@param[out] page   The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page already refers to a valid object
- \ref SU_ERROR_NO_DATA if this entity is not in a document or is on a shared
  layer
*/
LO_RESULT LOEntityGetPage(LOEntityRef entity, LOPageRef* page);

/**
@brief Gets whether or not this entity belongs to a group.
@param[in]  entity      The entity object.
@param[out] is_in_group Whether the entity is in a group.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_in_group is NULL
*/
LO_RESULT LOEntityIsInGroup(LOEntityRef entity, bool* is_in_group);

/**
@brief Gets the group that an entity belongs to.
@param[in]  entity The entity object.
@param[out] group  The group object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NO_DATA if entity is not in a group
- \ref SU_ERROR_NULL_POINTER_OUTPUT if group is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *group already refers to a valid object
*/
LO_RESULT LOEntityGetContainingGroup(LOEntityRef entity, LOGroupRef* group);

/**
@brief Moves an entity into a group. If the entity is already in a group, it
       will be removed from that group prior to being added to the new group.
       If this action results in the old group containing only one entity, the
       old group will be collapsed and the remaining entity will be moved to
       the old group's parent.
@param[in] entity The entity object.
@param[in] group  The group object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if entity is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if entity is locked
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_GENERIC if entity and group are not in the same document
- \ref SU_ERROR_GENERIC if entity and group belong to different pages, or if
  one is shared and the other is not
*/
LO_RESULT LOEntityMoveToGroup(LOEntityRef entity, LOGroupRef group);

/**
@brief Gets the style of an entity.
@param[in]  entity The entity object.
@param[out] style  The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
- \ref SU_ERROR_NO_DATA if entity is a group, and thus has no style
*/
LO_RESULT LOEntityGetStyle(LOEntityRef entity, LOStyleRef style);

/**
@brief Sets the style of an entity.
@param[in]  entity The entity object.
@param[out] style  The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if entity is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if entity is locked
*/
LO_RESULT LOEntitySetStyle(LOEntityRef entity, LOStyleRef style);
/**
@brief Gets whether or not an entity is on a shared layer. This function
       works for all entity types, including groups. Groups do not belong to a
       specific layer, but their children are all on either a shared or
       non-shared layer.
@param[in]  entity             The entity object.
@param[out] is_on_shared_layer Whether the entity is on a shared layer.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_on_shared_layer is NULL
- \ref SU_ERROR_NO_DATA if entity does not belong to a document
*/
LO_RESULT LOEntityIsOnSharedLayer(LOEntityRef entity, bool* is_on_shared_layer);

/**
@brief Gets whether an entity is locked.
@since LayOut 2018, API 3.0
@param[in]  entity    The entity object.
@param[out] is_locked Whether the entity is locked or unlocked.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_locked is NULL
*/
LO_RESULT LOEntityGetLocked(LOEntityRef entity, bool* is_locked);

/**
@brief Sets an entity to be locked or unlocked.
@since LayOut 2018, API 3.0
@param[in]  entity    The entity object.
@param[in]  lock      Whether the entity should be locked or unlocked.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
*/
LO_RESULT LOEntitySetLocked(LOEntityRef entity, bool lock);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_ENTITY_H_
