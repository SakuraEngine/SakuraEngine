// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_GROUP_H_
#define LAYOUT_MODEL_GROUP_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>
#include <LayOutAPI/model/document.h>

/**
@struct LOGroupRef
@brief References a group entity. A group is a special type of entity that
       does not belong to a layer and contains other entities as children. A
       group's children may include other groups, allowing for a hierarchical
       tree structure of entities. A group must contain at least one child
       and will be automatically collapsed if an operation is performed that
       results in the group being empty.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOGroupResizeBehaviorType
@brief Defines the different types of resize behavior when scale is changed.
*/
typedef enum {
  LOGroupResizeBehaviorType_None = 0,       /// Group is not resized.
  LOGroupResizeBehaviorType_Bounds,         /// Entity bounds are resized.
  LOGroupResizeBehaviorType_BoundsAndFonts  /// Bounds and fonts are resized.
} LOGroupResizeBehaviorType;

/**
@brief Creates a new group object, populating it with the entities in the given
       \ref LOEntityListRef. It is possible to create a group from entities
       that are already in a document, as well as from entities that are not
       yet in a document. For entities that are already in a document, they can
       only be grouped if they all belong to shared layers, or all belong to
       non-shared layers on the same page. If the entities are in a document,
       then the new group will be added to the document at the top of the group
       hierarchy.
@param[out] group       The group object.
@param[in]  entity_list The list of entities to add to the group.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if group is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *group already refers to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
- \ref SU_ERROR_GENERIC if entity_list is empty
- \ref SU_ERROR_GENERIC if entity_list contains entities that belong to
  different documents
- \ref SU_ERROR_GENERIC if entity_list contains a mix of entities that belong
  to a document and entities that don't belong to a document
- \ref SU_ERROR_GENERIC if entity_list contains entities on both shared
  and non-shared layers, or on non-shared layers belonging to different pages.
- \ref SU_ERROR_GENERIC if entity_list contains the same entity more than
  once.
*/
LO_RESULT LOGroupCreate(LOGroupRef* group, LOEntityListRef entity_list);

/**
@brief Adds a reference to a group object.
@param[in] group The group object.
@return
SU_ERROR_NONE on success
SU_ERROR_INVALID_INPUT if group does not refer to a valid object
*/
LO_RESULT LOGroupAddReference(LOGroupRef group);

/**
@brief Releases a group object. The object will be invalidated if
       releasing the last reference.
@param[in] group The group object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if group is NULL
- \ref SU_ERROR_INVALID_INPUT if *group does not refer to a valid object
*/
LO_RESULT LOGroupRelease(LOGroupRef* group);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOGroupRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOGroupRef.
@param[in] entity The entity object.
@return
- The converted \ref LOGroupRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOGroupRef LOGroupFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOGroupRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@param[in] group The group object.
@return
- The converted \ref LOEntityRef if group is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOGroupToEntity(LOGroupRef group);

/**
@brief Removes all the entities from a group and removes the empty group. The
       entities will become children of the group's parent. *group will be set
       to invalid by this function.
@param[in] group The group object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if group is NULL
- \ref SU_ERROR_INVALID_INPUT if *group does not refer to a valid object
- \ref SU_ERROR_ENTITY_LOCKED if group is locked
*/
LO_RESULT LOGroupUngroup(LOGroupRef* group);

/**
@brief Populates a \ref LOEntityListRef object with all the children of a group.
@param[in] group       The group object.
@param[in] entity_list The entity list object to populate with the group's
                       children.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOGroupGetEntities(LOGroupRef group, LOEntityListRef entity_list);

/**
@brief Gets the number of children within a group.
@param[in]  group        The group object.
@param[out] num_entities The number of children in the group.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_entities is NULL
*/
LO_RESULT LOGroupGetNumberOfEntities(LOGroupRef group, size_t* num_entities);

/**
@brief Gets the child at the given index from within a group.
@param[in]  group        The group object.
@param[in]  index        The index of the child entity to get.
@param[out] child_entity The child entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if child_entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *child_entity already refers to a valid
  object
*/
LO_RESULT LOGroupGetEntityAtIndex(LOGroupRef group, size_t index, LOEntityRef* child_entity);

/**
@brief Gets the scale factor set on a group.
@since LayOut 2018, API 3.0
@param[in]  group         The group object.
@param[out] scale_factor  The scale factor of the group.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if scale_context is NULL
- \ref SU_ERROR_NO_DATA if group does not have a scale factor set.
*/
LO_RESULT LOGroupGetScaleFactor(LOGroupRef group, double* scale_factor);

/**
@brief Sets the scale factor on a group.
@since LayOut 2018, API 3.0
@param[in] group           The group object.
@param[in] scale_factor    The scale factor to set.
@param[in] units           The units format to use.
@param[in] resize_behavior How entities in the group should adjust
                           to the new scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if group already effectively has a scale from
  a parent or sub-group.
- \ref SU_ERROR_LAYER_LOCKED if group is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if group is locked
*/
LO_RESULT LOGroupSetScaleFactor(
    LOGroupRef group, double scale_factor, LODocumentUnits units,
    LOGroupResizeBehaviorType resize_behavior);

/**
@brief Returns the scale units for a group.
@since LayOut 2018, API 3.0
@param[in]  group     The group object.
@param[out] units     The unit format for the group.
@param[out] precision The units precision. This is expressed as a value in the
                      current units.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if units is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if precision is NULL
- \ref SU_ERROR_NO_DATA if this specific group does not have a scale factor set.
*/
LO_RESULT LOGroupGetScaleUnits(LOGroupRef group, LODocumentUnits* units, double* precision);

/**
@brief Sets the units format on a group.
@since LayOut 2018, API 3.0
@param[in] group The group object.
@param[in] units The units format to use.
@param[in] precision The units precision. This is expressed as a value in the
                     specified units. LayOut only allows for a finite set of
                     precision values for each units setting, so it will set
                     the precision to the closest valid setting for the
                     specified units. See the "Units" section of LayOut's
                     "Document Setup" dialog for a reference of the available
                     precisions for each units setting.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if group is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if group is locked
- \ref SU_ERROR_UNSUPPORTED if group does not have a scale factor set.
*/
LO_RESULT LOGroupSetScaleUnits(LOGroupRef group, LODocumentUnits units, double precision);

/**
@brief Removes the scale factor of a group.
@since LayOut 2018, API 3.0
@param[in] group           The group object.
@param[in] resize_behavior How entities in the group should adjust
                           to the new scale.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if group is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if group is locked
- \ref SU_ERROR_UNSUPPORTED if group does not have a scale factor set.
*/
LO_RESULT LOGroupRemoveScaleFactor(LOGroupRef group, LOGroupResizeBehaviorType resize_behavior);

#ifdef __cplusplus
}  // end extern C
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_GROUP_H_
