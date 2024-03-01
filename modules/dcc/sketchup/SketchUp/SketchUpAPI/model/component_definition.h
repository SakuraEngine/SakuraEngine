// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUComponentDefinitionRef.
 */
#ifndef SKETCHUP_MODEL_COMPONENT_DEFINITION_H_
#define SKETCHUP_MODEL_COMPONENT_DEFINITION_H_

#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/curve.h>
#include <SketchUpAPI/model/defs.h>
#include <SketchUpAPI/model/model_version.h>
#include <time.h>

#pragma pack(push, 8)

#if defined(__APPLE__)
  // This is added to remove the missing declarations warning on the mac
  // for an enum defined inside a structure. [SUSnapToBehavior]
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wmissing-declarations"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUComponentDefinitionRef
@extends SUEntityRef
@brief  References a component definition.
*/

/**
@struct SUComponentBehavior
@brief  Describes how the component behaves in a SketchUp model, e.g. how it glues to surfaces.
*/
struct SUComponentBehavior {
  /**
  @enum SUSnapToBehavior
  @brief Describes how the component instance can be placed when a user adds
         it to a model. For example a window component instance should snap
         to a vertical plane.
  */
  enum SUSnapToBehavior {
    SUSnapToBehavior_None = 0,
    SUSnapToBehavior_Any,
    SUSnapToBehavior_Horizontal,
    SUSnapToBehavior_Vertical,
    SUSnapToBehavior_Sloped
  };

  /**
  How the component should snap to the surface where it's placed.
  */
  enum SUSnapToBehavior component_snap;

  /**
  Whether the component creates an opening when placed on a surface, e.g. a window frame component.
  */
  bool component_cuts_opening;

  /**
  Whether the component visually revolves around its Z axes to have its X-Z plane face the camera.
  */
  bool component_always_face_camera;

  /**
  Whether the component always casts a shadow as if it were facing the direction of the sun.
  */
  bool component_shadows_face_sun;

  /**
  Bitmask where set bits indicate which scale tool handles are hidden on a given component:

  - Bit0: disable scale along X axis,
  - Bit1: disable scale along Y axis,
  - Bit2: disable scale along Z axis,
  - Bit3: disable scale in X-Z plane,
  - Bit4: disable scale in Y-Z plane,
  - Bit5: disable scale in X-Y plane,
  - Bit6: disable scale uniform (XYZ)

  Prior to SketchUp 2018, API 6.0 this field existed but was never used.
  */
  size_t component_no_scale_mask;
};

/**
@enum SUComponentType
@brief Indicates the type of the component.
*/
enum SUComponentType {
  SUComponentType_Normal,  ///< Regular component definition
  SUComponentType_Group,   ///< Group definition
  SUComponentType_Image    ///< Image definition. (Added SketchUp 2022, API 10.0)
};

/**
@brief Converts from an \ref SUComponentDefinitionRef to an \ref SUEntityRef.
       This is essentially an upcast operation.
@param[in] comp_def The given component definition reference.
@related SUComponentDefinitionRef
@return
- The converted \ref SUEntityRef if comp_def is a valid component
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUComponentDefinitionToEntity(SUComponentDefinitionRef comp_def);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUComponentDefinitionRef.
       This is essentially a downcast operation so the given entity must be
       convertible to an \ref SUComponentDefinitionRef.
@param[in] entity The given entity reference.
@related SUComponentDefinitionRef
@return
- The converted \ref SUComponentDefinitionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUComponentDefinitionRef SUComponentDefinitionFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUComponentDefinitionRef to an \ref
       SUDrawingElementRef. This is essentially an upcast operation.
@param[in] comp_def The given component definition reference.
@related SUComponentDefinitionRef
@return
- The converted \ref SUEntityRef if comp_def is a valid component
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef
SUComponentDefinitionToDrawingElement(SUComponentDefinitionRef comp_def);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref
       SUComponentDefinitionRef. This is essentially a downcast operation so the
       given element must be convertible to an \ref SUComponentDefinitionRef.
@param[in] drawing_elem The given element reference.
@related SUComponentDefinitionRef
@return
- The converted \ref SUComponentDefinitionRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
SU_EXPORT SUComponentDefinitionRef
SUComponentDefinitionFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates a new component definition. The created definition must be
       released with SUComponentDefinitionRelease(), or attached to either
       a parent component or parent model. Add the new component definition to
       model using SUModelAddComponentDefinitions() before making any
       modifications to it. Once the component definition is owned by a model,
       use SUModelRemoveComponentDefinitions() to remove it.
@param[out] comp_def The component object created.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if comp_def is NULL
*/
SU_RESULT SUComponentDefinitionCreate(SUComponentDefinitionRef* comp_def);

/**
@brief Releases a component definition object and its associated resources. If
       the provided definition was contained by a model, use
       SUModelRemoveComponentDefinitions() to remove the definition and all
       instances.
@param[in] comp_def The component definition object.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if comp_def is NULL
- \ref SU_ERROR_INVALID_OUTPUT if comp_def points to an invalid object
*/
SU_RESULT SUComponentDefinitionRelease(SUComponentDefinitionRef* comp_def);

/**
@brief Retrieves the name of a component definition.
@param[in]  comp_def The component definition object.
@param[out] name     The name retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentDefinitionGetName(SUComponentDefinitionRef comp_def, SUStringRef* name);

/**
@brief Sets the name of a component definition.

@bug Before SketchUp 2022.0.1 (API 10.0) this would not ensure the definition name was unique.
  As of SketchUp 2022.0.1 the component will be given a unique name automatically if needed.

@param[in] comp_def The component definition object.
@param[in] name The name of the component definition. Assumed to be UTF-8
                encoded. If the requested name already belongs to another definition in the model,
                a unique name will be generated based on this name.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if \p name is NULL
*/
SU_RESULT SUComponentDefinitionSetName(SUComponentDefinitionRef comp_def, const char* name);

/**
@brief Retrieves the globally unique identifier (guid) string of a component
       definition.

@since SketchUp 2015, API 3.0

@see SUModelGetGuid
@see SUSkpReadGuid

@param[in]  comp_def The component definition object.
@param[out] guid_ref The guid retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if guid is NULL
- \ref SU_ERROR_INVALID_OUTPUT if guid does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentDefinitionGetGuid(SUComponentDefinitionRef comp_def, SUStringRef* guid_ref);

/**
@brief Retrieves the entities of the component definition.
@param[in]  comp_def The component definition object.
@param[out] entities The entities retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entities is NULL
*/
SU_RESULT SUComponentDefinitionGetEntities(
    SUComponentDefinitionRef comp_def, SUEntitiesRef* entities);

/**
@brief Retrieves the description of the component definition.
@param[in]  comp_def The component definition object.
@param[out] desc     The description retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if desc is NULL
- \ref SU_ERROR_INVALID_OUTPUT if desc does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentDefinitionGetDescription(SUComponentDefinitionRef comp_def, SUStringRef* desc);

/**
@brief Sets the description of the component definition.
@param[in] comp_def The component definition object.
@param[in] desc     The description to be set. Assumed to be UTF-8 encoded.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if desc is NULL
*/
SU_RESULT SUComponentDefinitionSetDescription(SUComponentDefinitionRef comp_def, const char* desc);

/**
@brief Create an instance of a component definition.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2018, API 6.0. In previous releases there was a
         recommendation to not release an instance created with this method if
         it was associated with a parent using SUEntitiesAddInstance(). The
         limitation was removed by generalizing SUComponentInstanceRelease()
         to correctly release instances whether or not they are contained in a
         parent component.

@related SUComponentDefinitionRef
@param[in]  comp_def The component definition object.
@param[out] instance The instance created.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instance is NULL
- \ref SU_ERROR_GENERIC if comp_def is not the definition of a type that can
  be instantiated
*/
SU_RESULT SUComponentDefinitionCreateInstance(
    SUComponentDefinitionRef comp_def, SUComponentInstanceRef* instance);

/**
@brief Retrieves the total number of instances of the provided definition.
       This method takes into account the full hierarchy of the model.
       Therefore, the count is influenced by adding/removing instances of other
       definitions which contain an instance of this definition. Users should
       not use this function to determine the count to be passed to \ref
       SUComponentDefinitionGetInstances specifying the number of instances to
       be retrieved.
@since SketchUp 2017, API 5.0
@param[in]  comp_def The component definition object.
@param[out] count    The number of instances of the definition.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUComponentDefinitionGetNumUsedInstances(
    SUComponentDefinitionRef comp_def, size_t* count);

/**
@brief Retrieves the number of unique instances of the provided definition. The
       returned count represents the number of instances of this definition in
       the model's root plus the number instances of this definition contained
       in other definitions.
@since SketchUp 2017, API 5.0
@param[in]  comp_def The component definition object.
@param[out] count    The number of instances of the definition.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUComponentDefinitionGetNumInstances(SUComponentDefinitionRef comp_def, size_t* count);

/**
@brief Retrieves the instances of the definition.
@since SketchUp 2017, API 5.0
@param[in]  comp_def  The component definition object.
@param[in]  len       The number of component instances to retrieve.
@param[out] instances The component instances retrieved.
@param[out] count     The number of component instances retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if instances or count is NULL
*/
SU_RESULT SUComponentDefinitionGetInstances(
    SUComponentDefinitionRef comp_def, size_t len, SUComponentInstanceRef instances[],
    size_t* count);

/**
@brief Retrieves the behavior of a component definition.
@param[in]  comp_def The component definition object.
@param[out] behavior The behavior retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
*/
SU_RESULT SUComponentDefinitionGetBehavior(
    SUComponentDefinitionRef comp_def, struct SUComponentBehavior* behavior);

/**
@brief Sets the component behavior of a component definition.
@param[in] comp_def The component definition object.
@param[in] behavior The behavior to set.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if behavior is NULL
*/
SU_RESULT SUComponentDefinitionSetBehavior(
    SUComponentDefinitionRef comp_def, const struct SUComponentBehavior* behavior);

/**
@brief Applies a schema type from a schema to a component definition.
@param[in] comp_def The component definition object.
@param[in] schema_ref The schema that owns the schema type to apply.
@param[in] schema_type_ref The schema type to apply.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_INVALID_INPUT if schema_ref is not a valid object
- \ref SU_ERROR_INVALID_INPUT if schema_type_ref is not a valid object
*/
SU_RESULT SUComponentDefinitionApplySchemaType(
    SUComponentDefinitionRef comp_def, SUSchemaRef schema_ref, SUSchemaTypeRef schema_type_ref);

/**
@brief Retrieves a flag indicating whether the component definition was created
       inside the current SketchUp model or whether it was added from another
       SKP file.
@since SketchUp 2016, API 4.0
@param[in]  comp_def    The component definition object.
@param[out] is_internal The bool value retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_internal is NULL
*/
SU_RESULT SUComponentDefinitionIsInternal(SUComponentDefinitionRef comp_def, bool* is_internal);

/**
@brief Retrieves the path where the component definition was loaded from.
@since SketchUp 2016, API 4.0
@param[in]  comp_def The component definition object.
@param[out] path     A valid path if successful.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if path does not point to a valid \ref
  SUStringRef object
*/
SU_RESULT SUComponentDefinitionGetPath(SUComponentDefinitionRef comp_def, SUStringRef* path);

/**
@brief Gets the load time of the component definition. For an internal component
       definition, this is the time that it was created. For an external
       component definition, this is the time that it was added to the model.
@since SketchUp 2016, API 4.0
@param[in]  comp_def  The component definition object.
@param[out] load_time The time value retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if load_time is NULL
*/
SU_RESULT SUComponentDefinitionGetLoadTime(SUComponentDefinitionRef comp_def, struct tm* load_time);

/**
@brief Retrieves the number of openings from the component definition.
@since SketchUp 2016, API 4.0
@param[in]  comp_def  The component definition object.
@param[out] count     The number of openings.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
*/
SU_RESULT SUComponentDefinitionGetNumOpenings(SUComponentDefinitionRef comp_def, size_t* count);

/**
@brief Retrieves the openings from the component definition. The openings
       retrieved must be released with SUOpeningRelease().
@since SketchUp 2016, API 4.0
@param[in]  comp_def  The component definition object.
@param[in]  len       The number of openings to retrieve.
@param[out] openings  The \ref SUOpeningRef objects retrieved.
@param[out] count     The number of openings retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if openings or count is NULL
*/
SU_RESULT SUComponentDefinitionGetOpenings(
    SUComponentDefinitionRef comp_def, size_t len, SUOpeningRef openings[], size_t* count);

/**
@brief Retrieves the insertion point from the component definition.
@since SketchUp 2016, API 4.0
@param[in]  comp_def  The component definition object.
@param[out] point     The insertion point retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if point is NULL
*/
SU_RESULT SUComponentDefinitionGetInsertPoint(
    SUComponentDefinitionRef comp_def, struct SUPoint3D* point);

/**
@brief Retrieves the \ref SUComponentType() from the component definition.
@note This function didn't recognize Image type definitions until SketchUp 2022.2, API 10.0.
@since SketchUp 2016, API 4.0
@param[in]  comp_def  The component definition object.
@param[out] type      The \ref SUComponentType() retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
SU_RESULT SUComponentDefinitionGetType(
    SUComponentDefinitionRef comp_def, enum SUComponentType* type);

/**
@brief Updates the faces in the component definition so that they are oriented
       consistently.
@since SketchUp 2016, API 4.0
@param[in] comp_def The component definition object.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def is invalid
*/
SU_RESULT SUComponentDefinitionOrientFacesConsistently(SUComponentDefinitionRef comp_def);

/**
@brief Sets the insertion point for the component definition.
@since SketchUp 2016, API 4.0

@deprecated As of SketchUp 2020.0 the insertion point cannot be changed.
  This function is now a NOOP.

@warning Breaking Change: The behavior of this method was changed in
         SketchUp 2018, API 6.0. In previous releases if the second argument was
         null this method returned \ref SU_ERROR_NULL_POINTER_OUTPUT, but this
         was changed to \ref SU_ERROR_NULL_POINTER_INPUT for consistency with
         other API methods.

@param[in] comp_def  The component definition object.
@param[in] point     The \ref SUPoint3D to use.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE
*/
SU_DEPRECATED_FUNCTION("SketchUp API 8.0")
SU_RESULT SUComponentDefinitionSetInsertPoint(
    SUComponentDefinitionRef comp_def, const struct SUPoint3D* point);

/**
@brief Sets the axes of the component definition.
@since SketchUp 2016, API 4.0
@param[in] comp_def The component definition object.
@param[in] axes     The \ref SUAxesRef to use.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if comp_def or axes are invalid
*/
SU_RESULT SUComponentDefinitionSetAxes(SUComponentDefinitionRef comp_def, SUAxesRef axes);

/**
@brief Retrieves a flag indicating whether the component definition is a Live
  Component or a sub-definition of a Live Component.

@note These components are parametrically generated and API users should not
  modify them.

@since SketchUp 2021.0, API9.0

@param[in]  comp_def  The component definition object.
@param[out] is_live   The bool value retrieved.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p comp_def is invalid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p is_live is NULL
*/
SU_RESULT SUComponentDefinitionIsLiveComponent(SUComponentDefinitionRef comp_def, bool* is_live);

/**
@brief Saves the component definition to a file.
@since SketchUp 2022.0, API 10.0
@param[in] comp_def  The component definition object.
@param[in] file_path The file path destination of the serialization operation.
                     Assumed to be UTF-8 encoded.
@param[in] version   The SKP file format version to use when saving.
                     Use \ref SUModelVersion_Current for the current version.
@related SUComponentDefinitionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p comp_def is not a valid object
- \ref SU_ERROR_INVALID_ARGUMENT if \p comp_def is not owned by a model
- \ref SU_ERROR_MODEL_VERSION if \p version is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if \p file_path is NULL
- \ref SU_ERROR_SERIALIZATION if the serialization operation itself fails
*/
SU_RESULT SUComponentDefinitionSaveToFile(
    SUComponentDefinitionRef comp_def, const char* file_path, enum SUModelVersion version);

#ifdef __cplusplus
}  // extern "C"
#endif

#if defined(__APPLE__)
  #pragma GCC diagnostic pop
#endif

#pragma pack(pop)

#endif  // SKETCHUP_MODEL_COMPONENT_DEFINITION_H_
