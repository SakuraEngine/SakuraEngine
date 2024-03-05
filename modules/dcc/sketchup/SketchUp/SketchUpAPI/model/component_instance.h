// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUComponentInstanceRef.
 */
#ifndef SKETCHUP_MODEL_COMPONENT_INSTANCE_H_
#define SKETCHUP_MODEL_COMPONENT_INSTANCE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/transformation.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUComponentInstanceRef
@extends SUDrawingElementRef
@brief  References a component instance, i.e. an instance of a component
        definition.
*/

/**
@brief Converts from an \ref SUComponentInstanceRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] instance The given component instance reference.
@related SUComponentInstanceRef
@return
- The converted \ref SUEntityRef if instance is a valid component instance
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUComponentInstanceToEntity(SUComponentInstanceRef instance);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUComponentInstanceRef.
       This is essentially a downcast operation so the given entity must be
       convertible to a component instance.
@param[in] entity The given entity reference.
@related SUComponentInstanceRef
@return
- The converted \ref SUComponentInstanceRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUComponentInstanceRef SUComponentInstanceFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUComponentInstanceRef to an \ref
       SUDrawingElementRef. This is essentially an upcast operation.
@param[in] instance The given component instance reference.
@related SUComponentInstanceRef
@return
- The converted \ref SUEntityRef if instance is a valid component instance
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUComponentInstanceToDrawingElement(SUComponentInstanceRef instance);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref
       SUComponentInstanceRef. This is essentially a downcast operation so the
       given element must be convertible to a component instance.
@param[in] drawing_elem The given drawing element reference.
@related SUComponentInstanceRef
@return
- The converted \ref SUComponentInstanceRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUComponentInstanceRef
SUComponentInstanceFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Sets the name of a component instance object.
@param[in] instance The component instance object.
@param[in] name     The name string to set the component instance object.
                    Assumed to be UTF-8 encoded.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SUComponentInstanceSetName(SUComponentInstanceRef instance, const char* name);

/**
@brief Deallocates a component instance object created with
       SUComponentDefinitionCreateInstance().
@param[in] instance The component instance object.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instance is NULL
*/
SU_RESULT SUComponentInstanceRelease(SUComponentInstanceRef* instance);

/**
@brief Retrieves the name of a component instance object.
@param[in]  instance The component instance object.
@param[out] name     The name retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentInstanceGetName(SUComponentInstanceRef instance, SUStringRef* name);

/**
@brief Sets the globally unique identifier (guid) string of a instance object.
@since SketchUp 2015, API 3.0
@param[in]  instance The component instance object.
@param[in]  guid     The utf-8 formatted guid string.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_INVALID_INPUT if guid is NULL or invalid
*/
SU_RESULT SUComponentInstanceSetGuid(SUComponentInstanceRef instance, const char* guid);

/**
@brief Retrieves the globally unique identifier (guid) string of a instance object.
@param[in]  instance The component instance object.
@param[out] guid     The guid retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guid is NULL
- \ref SU_ERROR_INVALID_OUTPUT if guid does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentInstanceGetGuid(SUComponentInstanceRef instance, SUStringRef* guid);

/**
@brief Sets the transform of a component instance object.

The transform is relative to the parent component. If the parent component is
the root component of a model, then the transform is relative to absolute
coordinates.
@param[in] instance  The component instance object.
@param[in] transform The affine transform to set.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if transform is NULL
*/
SU_RESULT SUComponentInstanceSetTransform(
    SUComponentInstanceRef instance, const struct SUTransformation* transform);

/**
@brief Retrieves the transform of a component instance object.

See description of \ref SUComponentInstanceSetTransform() for a discussion of
component instance transforms.
@param[in]  instance  The component instance object.
@param[out] transform The transform retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if transform is NULL
*/
SU_RESULT SUComponentInstanceGetTransform(
    SUComponentInstanceRef instance, struct SUTransformation* transform);

/**
@brief Retrieves the component definition of a component instance object.
@param[in]  instance  The component instance object.
@param[out] component The component definition retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p component is NULL
*/
SU_RESULT SUComponentInstanceGetDefinition(
    SUComponentInstanceRef instance, SUComponentDefinitionRef* component);

/**
@brief Locks the instance if is_locked is true, otherwise unlocks the instance.
@since SketchUp 2016, API 4.0
@param[in] instance The instance object.
@param[in] lock     if true lock the instance, otherwise unlock it.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_locked is NULL
*/
SU_RESULT SUComponentInstanceSetLocked(SUComponentInstanceRef instance, bool lock);

/**
@brief Retrieves a boolean indicating whether tne instance is locked.
@since SketchUp 2016, API 4.0
@param[in]  instance  The instance object.
@param[out] is_locked returns true if the instance is locked
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_locked is NULL
*/
SU_RESULT SUComponentInstanceIsLocked(SUComponentInstanceRef instance, bool* is_locked);

/**
@brief  Saves the component instance data to a file.
@param[in] instance  The component instance object.
@param[in] file_path The file path destination of the serialization operation.
                     Assumed to be UTF-8 encoded.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_SERIALIZATION if the serialization operation itself fails
@deprecated Superseded by SUComponentDefinitionSaveToFile(). This function had the side effects of
  making the instance unique and changing its transformation. An SKP file also conceptually maps
  better to a component definition than a component instance.
*/
SU_DEPRECATED_FUNCTION("10.0")
SU_RESULT SUComponentInstanceSaveAs(SUComponentInstanceRef instance, const char* file_path);

/**
@brief Computes the volume of the component instance.
@since SketchUp 2016, API 4.0
@param[in]  instance  The component instance object.
@param[in]  transform A transformation to be applied to the component instance.
                      If set to NULL, the volume will be computed based on the
                      instance's transformation. Note that in this case if the
                      instance is contained within another instance or group,
                      the parent transformation is not factored in.
@param[out] volume    The volume of the component instance in cubic inches.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if component instance is not a valid object
- \ref SU_ERROR_NO_DATA if component instance is not manifold.
- \ref SU_ERROR_NULL_POINTER_OUTPUT if volume is NULL
*/
SU_RESULT SUComponentInstanceComputeVolume(
    SUComponentInstanceRef instance, const struct SUTransformation* transform, double* volume);

/**
@brief Creates a \ref SUDynamicComponentInfoRef object.
@since SketchUp 2016, API 4.0
@param[in]  instance The component instance object.
@param[out] dc_info  The dynamic component info object.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NO_DATA if instance is not a dynamic component
- \ref SU_ERROR_NULL_POINTER_OUTPUT if dc_info is NULL
- \ref SU_ERROR_OVERWRITE_VALID if dc_info is a valid object
*/
SU_RESULT SUComponentInstanceCreateDCInfo(
    SUComponentInstanceRef instance, SUDynamicComponentInfoRef* dc_info);

/**
@brief Creates a \ref SUClassificationInfoRef object.
@since SketchUp 2017, API 5.0
@param[in]  instance            The component instance object.
@param[out] classification_info The classification info object.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is not a valid object
- \ref SU_ERROR_NO_DATA if instance is not a classified object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if classification_info is NULL
*/
SU_RESULT SUComponentInstanceCreateClassificationInfo(
    SUComponentInstanceRef instance, SUClassificationInfoRef* classification_info);

/**
@brief Retrieves the number of attached component instances.
@param[in]  instance The component instance object.
@param[out] count    The number of attached instances.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUComponentInstanceGetNumAttachedInstances(
    SUComponentInstanceRef instance, size_t* count);

/**
@brief Retrieves the attached component instances.
@param[in]  instance  The component instance object.
@param[in]  len       The number of instances to retrieve.
@param[out] instances The attached instances retrieved. These may be instances
                      or groups.
@param[out] count     The number of instances retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instances or count is NULL
*/
SU_RESULT SUComponentInstanceGetAttachedInstances(
    SUComponentInstanceRef instance, size_t len, SUComponentInstanceRef instances[], size_t* count);

/**
@brief Retrieves the number of drawing element this instance is attached to.
@since SketchUp 2018, API 6.0
@param[in]  instance The component instance object.
@param[out] count    The number of drawing elements this component instance is
                     attached to.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUComponentInstanceGetNumAttachedToDrawingElements(
    SUComponentInstanceRef instance, size_t* count);

/**
@brief Retrieves the drawing elements this instance is attached to.
@since SketchUp 2018, API 6.0
@param[in]  instance The component instance object.
@param[in]  len      The number of instances to retrieve.
@param[out] elements The drawing elements retrieved. These may be instances,
                     groups or faces.
@param[out] count    The number of drawing elements retrieved.
@related SUComponentInstanceRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if instance is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instances or count is NULL
*/
SU_RESULT SUComponentInstanceGetAttachedToDrawingElements(
    SUComponentInstanceRef instance, size_t len, SUDrawingElementRef elements[], size_t* count);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_COMPONENT_INSTANCE_H_
