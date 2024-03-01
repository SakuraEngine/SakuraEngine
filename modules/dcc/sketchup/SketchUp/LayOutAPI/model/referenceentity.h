// Copyright 2022 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_REFERENCEENTITY_H_
#define LAYOUT_MODEL_REFERENCEENTITY_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOReferenceEntityRef
@brief References an entity containing the contents of an imported file.
@since LayOut 2023, API 8.0
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
@brief Adds a reference to a reference entity object.
@since LayOut 2023, API 8.0
@param[in] reference_entity The reference entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if reference_entity does not refer to a valid object
*/
LO_RESULT LOReferenceEntityAddReference(LOReferenceEntityRef reference_entity);

/**
@brief Releases a reference entity object. The object will be invalidated if releasing the last
       reference.
@since LayOut 2023, API 8.0
@param[in] reference_entity The reference entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if reference_entity is NULL
- \ref SU_ERROR_INVALID_INPUT if *reference_entity does not refer to a valid object
*/
LO_RESULT LOReferenceEntityRelease(LOReferenceEntityRef* reference_entity);

/**
@brief Converts from a \ref LOEntityRef to a \ref LOReferenceEntityRef.
       This is essentially a downcast operation so the given \ref LOEntityRef
       must be convertible to a \ref LOReferenceEntityRef.
@since LayOut 2023, API 8.0
@param[in] entity The entity object.
@return
- The converted \ref LOReferenceEntityRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOReferenceEntityRef LOReferenceEntityFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOReferenceEntityRef to a \ref LOEntityRef.
       This is essentially an upcast operation.
@since LayOut 2023, API 8.0
@param[in] reference_entity The reference entity object.
@return
- The converted \ref LOEntityRef if reference_entity is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOReferenceEntityToEntity(LOReferenceEntityRef reference_entity);

/**
@brief Returns any clip mask assigned to the reference entity.
@since LayOut 2023, API 8.0
@param[in]  reference_entity The reference entity object.
@param[out] clip_mask        The clip mask of the reference entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if reference_entity does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if clip_mask is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *clip_mask already refers to a valid object
- \ref SU_ERROR_NO_DATA if the reference entity is not being clipped by a clip mask
*/
LO_RESULT LOReferenceEntityGetClipMask(
    LOReferenceEntityRef reference_entity, LOEntityRef* clip_mask);

/**
@brief Sets the clip mask of the reference entity. A clip mask defines a region of the entity that
is visible. This allows you to crop with arbitrary shapes. This operation will replace any clip mask
that is already assigned to this reference entity. The entity being used must not be already part of
a document or group. The clip mask entity must be either a rectangle, ellipse or a path, or may be
SU_INVALID, which will remove the existing clip mask, if any.
@since LayOut 2023, API 8.0
@param[in] reference_entity The reference entity object.
@param[in] clip_mask        The new clip mask for the reference entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if reference_entity does not refer to a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if clip_mask is already in a document or group
- \ref SU_ERROR_UNSUPPORTED if clip_mask is not a rectangle, ellipse, or path
- \ref SU_ERROR_LAYER_LOCKED if reference_entity is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if reference_entity is locked
*/
LO_RESULT LOReferenceEntitySetClipMask(
    LOReferenceEntityRef reference_entity, LOEntityRef clip_mask);

/**
@brief Creates the entities that represent the reference entity in its exploded form and adds them
to a \ref LOEntityListRef. It is NOT necessary to explicitly release these entities, since \ref
       LOEntityListRef itself adds a reference to the entities and will release them when they are
       removed from the list or when the list is released.
@since LayOut 2023, API 8.0
@param[in] reference_entity The reference entity object.
@param[in] entity_list      The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if reference_entity does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOReferenceEntityGetExplodedEntities(
    LOReferenceEntityRef reference_entity, LOEntityListRef entity_list);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus
#endif  // LAYOUT_MODEL_REFERENCEENTITY_H_
