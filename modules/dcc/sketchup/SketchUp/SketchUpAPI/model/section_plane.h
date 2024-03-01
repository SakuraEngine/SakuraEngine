// Copyright 2015 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUSectionPlaneRef.
 */
#ifndef SKETCHUP_MODEL_SECTION_PLANE_H_
#define SKETCHUP_MODEL_SECTION_PLANE_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/geometry.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUSectionPlaneRef
@extends SUDrawingElementRef
@brief  A sectionPlane entity reference.
@since SketchUp 2016, API 4.0
*/

/**
@brief Converts from an \ref SUSectionPlaneRef to an \ref SUEntityRef. This is
       essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] sectionPlane The sectionPlane object.
@related SUSectionPlaneRef
@return
- The converted \ref SUEntityRef if sectionPlane is a valid object
- If not, the returned reference will be invalid
*/
SU_EXPORT SUEntityRef SUSectionPlaneToEntity(SUSectionPlaneRef sectionPlane);

/**
@brief Converts from an \ref SUEntityRef to an \ref SUSectionPlaneRef.
       This is essentially a downcast operation so the given \ref SUEntityRef
       must be convertible to an \ref SUSectionPlaneRef.
@since SketchUp 2016, API 4.0
@param[in] entity The entity object.
@related SUSectionPlaneRef
@return
- The converted \ref SUSectionPlaneRef if the downcast operation succeeds
- If the downcast operation fails, the returned reference will be invalid
*/
SU_EXPORT SUSectionPlaneRef SUSectionPlaneFromEntity(SUEntityRef entity);

/**
@brief Converts from an \ref SUSectionPlaneRef to an \ref SUDrawingElementRef.
       This is essentially an upcast operation.
@since SketchUp 2016, API 4.0
@param[in] sectionPlane The given sectionPlane reference.
@related SUSectionPlaneRef
@return
- The converted \ref SUEntityRef if sectionPlane is a valid sectionPlane
- If not, the returned reference will be invalid
*/
SU_EXPORT SUDrawingElementRef SUSectionPlaneToDrawingElement(SUSectionPlaneRef sectionPlane);

/**
@brief Converts from an \ref SUDrawingElementRef to an \ref SUSectionPlaneRef.
       This is essentially a downcast operation so the given element must be
       convertible to an \ref SUSectionPlaneRef.
@since SketchUp 2016, API 4.0
@param[in] drawing_elem The given element reference.
@related SUSectionPlaneRef
@return
- The converted \ref SUSectionPlaneRef if the downcast operation succeeds
- If not, the returned reference will be invalid.
*/
SU_EXPORT SUSectionPlaneRef SUSectionPlaneFromDrawingElement(SUDrawingElementRef drawing_elem);

/**
@brief Creates an sectionPlane object. The sectionPlane object must be
       subsequently deallocated with \ref SUSectionPlaneRelease() unless it is
       associated with a parent object.  The plane is initialized as an xy
       plane and can be changed with the \ref SUSectionPlaneSetPlane().
@since SketchUp 2016, API 4.0
@param[in] sectionPlane The sectionPlane object.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if sectionPlane is NULL
- \ref SU_ERROR_OVERWRITE_VALID if sectionPlane references a valid object
*/
SU_RESULT SUSectionPlaneCreate(SUSectionPlaneRef* sectionPlane);

/**
@brief Releases a sectionPlane object. The sectionPlane object must have been
       created with \ref SUSectionPlaneCreate() and not subsequently associated
       with a parent object (e.g. \ref SUEntitiesRef).
@since SketchUp 2016, API 4.0
@param[in] sectionPlane The sectionPlane object.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if sectionPlane is NULL
- \ref SU_ERROR_INVALID_INPUT if sectionPlane does not reference a valid object
*/
SU_RESULT SUSectionPlaneRelease(SUSectionPlaneRef* sectionPlane);

/**
@brief Sets the plane of the section plane.
@param[in] sectionPlane The sectionPlane object.
@param[in] plane        The 3d plane to be set.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if plane is NULL
*/
SU_RESULT SUSectionPlaneSetPlane(SUSectionPlaneRef sectionPlane, const struct SUPlane3D* plane);

/**
@brief Retrieves the plane of the section plane.
@param[in]  sectionPlane  The sectionPlane object.
@param[out] plane         The 3d plane retrieved.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if plane is NULL
*/
SU_RESULT SUSectionPlaneGetPlane(SUSectionPlaneRef sectionPlane, struct SUPlane3D* plane);

/**
@brief Retrieves a boolean indicating whether or not the section plane is
       active.
@param[in]  sectionPlane  The sectionPlane object.
@param[out] is_active     Returns true if the section plane is active.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_active is NULL
*/
SU_RESULT SUSectionPlaneIsActive(SUSectionPlaneRef sectionPlane, bool* is_active);

/**
@brief Retrieves the name of a section plane object.
@since SketchUp 2018, API 6.0
@param[in]  sectionPlane  The section plane object.
@param[out] name          The name retrieved.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUSectionPlaneGetName(SUSectionPlaneRef sectionPlane, SUStringRef* name);

/**
@brief Sets the name of a section plane object.
@since SketchUp 2018, API 6.0
@param[in] sectionPlane The section plane object.
@param[in] name         The string to set as the section plane name.
                        Assumed to be UTF-8 encoded. An example of a name would
                        be "South West Section" for the section on the south
                        west side of a building.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
*/
SU_RESULT SUSectionPlaneSetName(SUSectionPlaneRef sectionPlane, const char* name);

/**
@brief Retrieves the symbol of a section plane object. The symbol is used in
       the Outliner and in the section display in the model. For example, you
       might have several sections on the same area of a building all named
       "South West Section" and use symbols to differenciate each section,
       "01", "02", "03".
@since SketchUp 2018, API 6.0
@param[in]  sectionPlane  The section plane object.
@param[out] symbol        The symbol retrieved.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if symbol is NULL
- \ref SU_ERROR_INVALID_OUTPUT if symbol does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUSectionPlaneGetSymbol(SUSectionPlaneRef sectionPlane, SUStringRef* symbol);

/**
@brief Sets the symbol of a section plane object.
@since SketchUp 2018, API 6.0
@param[in] sectionPlane The section plane object.
@param[in] symbol       The string to set as the section plane symbol.
                        Assumed to be UTF-8 encoded.  The maximum number of
                        characters is 3.
@related SUSectionPlaneRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if sectionPlane is not a valid object
- \ref SU_ERROR_INVALID_INPUT if symbol is greater than three characters long
- \ref SU_ERROR_NULL_POINTER_INPUT if symbol is NULL
*/
SU_RESULT SUSectionPlaneSetSymbol(SUSectionPlaneRef sectionPlane, const char* symbol);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_SECTION_PLANE_H_