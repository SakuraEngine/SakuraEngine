// Copyright 2016 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUInstancePathRef.
 */
#ifndef SKETCHUP_MODEL_INSTANCEPATH_H_
#define SKETCHUP_MODEL_INSTANCEPATH_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUInstancePathRef
@brief  An instance path type that provides a wrapping of a data structure
        of component instances.
*/

/**
@brief Creates an instance path object.
@param[out] instance_path The instance path object created.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if instance_path is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *instance_path refers to a valid object
*/
SU_RESULT SUInstancePathCreate(SUInstancePathRef* instance_path);

/**
@brief Creates a copy of an instance path object.
@param[out] instance_path The copy of instance path object.
@param[in]  source_path   The instance path to be copied.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if instance_path is NULL
- \ref SU_ERROR_INVALID_INPUT if source_path is not a valid object
- \ref SU_ERROR_OVERWRITE_VALID if *instance_path refers to a valid object
*/
SU_RESULT SUInstancePathCreateCopy(SUInstancePathRef* instance_path, SUInstancePathRef source_path);

/**
@brief Releases an instance path object.
@param[in] instance_path The instance path being released.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if instance_path is NULL
*/
SU_RESULT SUInstancePathRelease(SUInstancePathRef* instance_path);

/**
@brief Pushes a \ref SUComponentInstanceRef to an \ref SUInstancePathRef.
@param[in] instance_path      The instance path object.
@param[in] component_instance The component instance object.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path or component_instance is not a
  valid object
*/
SU_RESULT SUInstancePathPushInstance(
    SUInstancePathRef instance_path, SUComponentInstanceRef component_instance);

/**
@brief Pops the last \ref SUComponentInstanceRef from an \ref SUInstancePathRef.
@param[in] instance_path The instance path object.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if instance_path is NULL
*/
SU_RESULT SUInstancePathPopInstance(SUInstancePathRef instance_path);

/**
@brief Sets a \ref SUEntityRef to an \ref SUInstancePathRef.
@param[in] instance_path The instance path object.
@note Since SketchUp 2023.1, API version 11.1 you can unset the leaf entity by passing an SU_INVALID
        entity ref.
@param[in] entity        The entity to be set as a leaf in instance path or SU_INVALID if the
        leaf should be removed.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if instance_path is NULL
*/
SU_RESULT SUInstancePathSetLeaf(SUInstancePathRef instance_path, SUEntityRef entity);

/**
@brief Gets a path depth for \ref SUInstancePathRef.
       It only counts the component instances in the path, so the leaf node is
       not counted.
@param[in]  instance_path The instance path object.
@param[out] depth         The depth of instance path object.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if depth is NULL
*/
SU_RESULT SUInstancePathGetPathDepth(SUInstancePathRef instance_path, size_t* depth);

/**
@brief Gets the full path depth (including the leaf) for \ref SUInstancePathRef.
@param[in]  instance_path The instance path object.
@param[out] full_depth    The depth of instance path object including the leaf
                          (if it exists).
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if full_depth is NULL
*/
SU_RESULT SUInstancePathGetFullDepth(SUInstancePathRef instance_path, size_t* full_depth);

/**
@brief Gets the transform for \ref SUInstancePathRef.
@param[in]  instance_path The instance path object.
@param[out] transform     The transform from instance path.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUInstancePathGetTransform(
    SUInstancePathRef instance_path, struct SUTransformation* transform);

/**
@brief Gets the transform up to depth level for \ref SUInstancePathRef.
@param[in]  instance_path The instance path object.
@param[in]  depth         The depth for getting transforms up to.
@param[out] transform     The transform from instance path.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if depth exceeds the depth of instance_path
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUInstancePathGetTransformAtDepth(
    SUInstancePathRef instance_path, size_t depth, struct SUTransformation* transform);

/**
@brief Gets a component instance up to path depth level.
@param[in]  instance_path The instance path object.
@param[in]  depth         The depth for getting drawing element up to.
@param[out] instance      The component instance from instance path.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if depth exceeds the depth of instance_path
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instance is NULL
*/
SU_RESULT SUInstancePathGetInstanceAtDepth(
    SUInstancePathRef instance_path, size_t depth, SUComponentInstanceRef* instance);

/**
@brief Gets a leaf from an instance path as an entity object.
@param[in]  instance_path The instance path object.
@param[out] entity        The leaf from an instance path.
@related SUInstancePathRef
@return
- The leaf from instance path on success
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
*/
SU_RESULT SUInstancePathGetLeafAsEntity(SUInstancePathRef instance_path, SUEntityRef* entity);

/**
@brief Gets a leaf from an entity path as a drawing element object.
@param[in]  instance_path   The instance path object.
@param[out] drawing_element The leaf from an instance path.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
*/
SU_RESULT SUInstancePathGetLeaf(
    SUInstancePathRef instance_path, SUDrawingElementRef* drawing_element);

/**
@brief Validates an instance path.
@param[in]  instance_path The instance path object.
@param[out] valid         Whether the instance path is valid or not.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if valid is NULL
*/
SU_RESULT SUInstancePathIsValid(SUInstancePathRef instance_path, bool* valid);

/**
@brief Checks if an instance path is empty.
@param[in]  instance_path The instance path object.
@param[out] empty         Whether the instance path is empty or not.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if empty is NULL
*/
SU_RESULT SUInstancePathIsEmpty(SUInstancePathRef instance_path, bool* empty);

/**
@brief Checks if instance path contains a particular entity.
@param[in]  instance_path The instance path object.
@param[in]  entity        The entity object.
@param[out] contains      Whether the instance path contains the entity or not.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance_path_ref
                              or entity_ref is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if contains is NULL
*/
SU_RESULT SUInstancePathContains(
    SUInstancePathRef instance_path, SUEntityRef entity, bool* contains);
/**
@brief Retrieves the full persistent id for a given instance path.
@param[in]  instance_path  The instance path.
@param[out] pid            The persistent id.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_UNSUPPORTED if persistent id functionality is unsupported
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_INVALID_OUTPUT if pid is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pid is NULL
*/
SU_RESULT SUInstancePathGetPersistentID(SUInstancePathRef instance_path, SUStringRef* pid);

/**
@brief Retrieves the persistent id of an entity up to depth level in a given
       instance path.
@param[in]  instance_path  The instance path.
@param[in]  depth          The depth for getting persistent id up to.
@param[out] pid            The persistent id.
@related SUInstancePathRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_UNSUPPORTED if persistent id functionality is unsupported
- \ref SU_ERROR_INVALID_INPUT if instance_path is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if depth exceeds the depth of instance_path
- \ref SU_ERROR_INVALID_OUTPUT if pid is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if pid is NULL
*/
SU_RESULT SUInstancePathGetPersistentIDAtDepth(
    SUInstancePathRef instance_path, size_t depth, SUStringRef* pid);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_INSTANCEPATH_H_
