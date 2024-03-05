// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUGroupRef.
 */
#ifndef SKETCHUP_MODEL_GROUP_H_
#define SKETCHUP_MODEL_GROUP_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUGroupRef
@extends SUComponentInstanceRef
@brief  References a group object.
*/

/**
@brief Converts from an \ref SUGroupRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] group The given group reference.
@related SUGroupRef
@return
- The converted \ref SUEntityRef if group is a valid group
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUGroupToEntity(SUGroupRef group);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUGroupRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an SUGroupRef.
@param[in] entity The given entity reference.
@related SUGroupRef
@return
- The converted \ref SUGroupRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUGroupRef SUGroupFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUGroupRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@param[in] group The given group reference.
@related SUGroupRef
@return
- The converted \ref SUDrawingElementRef if group is a valid group
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUGroupToDrawingElement(SUGroupRef group);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUGroupRef.
       This is essentially a downcast operation so the given element must be
       convertible to an SUGroupRef.
@param[in] drawing_elem The given element reference.
@related SUGroupRef
@return
- The converted \ref SUGroupRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUGroupRef SUGroupFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Converts from an \ref SUGroupRef to an \ref SUComponentInstanceRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] group The given group reference.
@related SUGroupRef
@return
- The converted \ref SUComponentInstanceRef if group is a valid group
- If not, the returned reference will be invalid
*/
SU_EXPORT SUComponentInstanceRef SUGroupToComponentInstance(SUGroupRef group);

/**
@brief Converts from an \ref SUComponentInstanceRef to an \ref SUGroupRef. This
       is essentially a downcast operation so the given element must be
       convertible to an SUGroupRef.
@since SketchUp 2016, API 4.0
@param[in] component_inst The given component instance reference.
@related SUGroupRef
@return
- The converted \ref SUGroupRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUGroupRef SUGroupFromComponentInstance(SUComponentInstanceRef component_inst);

/**
@brief Creates a new group object.

The created group must be subsequently added to the Entities of a model,
component definition or a group. Use SUModelRemoveComponentDefinitions() to
remove the group from a model.
@param[out] group The group object created.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if group is NULL
*/
SU_RESULT SUGroupCreate(SUGroupRef* group);

/**
@brief Sets the name of a group object.
@param[in] group The group object.
@param[in] name  The name string to set the group object.
                 Assumed to be UTF-8 encoded.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SUGroupSetName(SUGroupRef group, const char* name);

/**
@brief Retrieves the name of a group object.
@param[in]  group The group object.
@param[out] name  The name retrieved.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUGroupGetName(SUGroupRef group, SUStringRef* name);

/**
@brief Retrieves the globally unique identifier (guid) string of a group object.
@since SketchUp 2015, API 3.0
@param[in]  group   The group object.
@param[out] guid    The guid retrieved.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guid is NULL
- \ref SU_ERROR_INVALID_OUTPUT if guid does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUGroupGetGuid(SUGroupRef group, SUStringRef* guid);

/**
@brief Sets the globally unique identifier (guid) string of a group object.
@since SketchUp 2015, API 3.0
@param[in]  group    The group object.
@param[in]  guid_str The utf-8 formatted guid string.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_INVALID_INPUT if guid is NULL or invalid
*/
SU_RESULT SUGroupSetGuid(SUGroupRef group, const char* guid_str);

/**
@brief Sets the transform of a group object.

The transform is relative to the parent component. If the parent component is
the root component of a model, then the transform is relative to absolute
coordinates.
@param[in] group     The group object.
@param[in] transform The affine transform to set.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object.
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL.
*/
SU_RESULT SUGroupSetTransform(SUGroupRef group, const struct SUTransformation* transform);

/**
@brief Retrieves the transform of a group object.

See description of SUGroupSetTransform for a discussion of group transforms.
@param[in]  group     The group object.
@param[out] transform The transform retrieved.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUGroupGetTransform(SUGroupRef group, struct SUTransformation* transform);

/**
@brief Retrieves the entities of the group object.
@param[in]  group    The group object.
@param[out] entities The entities retrieved.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if group is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entities is NULL
- \ref SU_ERROR_OVERWRITE_VALID if entities reference object is valid
*/
SU_RESULT SUGroupGetEntities(SUGroupRef group, SUEntitiesRef* entities);

/* The cond/endcond block will ensure that any code or comment keywords will be
ignored by the Doxygen parser and will not appear in any documentation */
/**@cond
@brief Retrieves the component definition of a group object.
@since SketchUp 2014, API 2.0
@param[in]  group     The group object.
@param[out] component The component definition retrieved.
@related SUGroupRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p group is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p component is NULL
- \ref SU_ERROR_OVERWRITE_VALID if \p component already refers to a valid object
*/
SU_RESULT SUGroupGetDefinition(SUGroupRef group, SUComponentDefinitionRef* component);

///**
//@brief Explodes a group into separate entities.
//@since SketchUp 2016, API 4.0
//@param[in]  group    The group object.
//@param[out] entities Array of entity pointers returned
//@related SUGroupRef
//@return
//- \ref SU_ERROR_NONE on success
//- \ref SU_ERROR_INVALID_INPUT if group is invalid
//- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_locked is NULL
// SUStringRef object
//*/
// SU_RESULT SUGroupExplode(SUGroupRef group, bool* is_locked);

/** @endcond */

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_GROUP_H_
