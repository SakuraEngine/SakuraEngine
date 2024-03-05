// Copyright 2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SUSelectionRef.
 */
#ifndef SKETCHUP_MODEL_SELECTION_H_
#define SKETCHUP_MODEL_SELECTION_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/model/defs.h>

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUSelectionRef
@brief  References a the model selection. This is only available from within the
        SketchUp application.
*/

/**
@brief Used with SUSelectionIsType() to query for what type the selection is.
@see SUSelectionIsType()
@since SketchUp 2020.2, API 8.2
*/
enum SUSelectionType {
  SUSelectionType_Curve,        ///< Query for the selection to contain a single
                                ///< SUCurveRef or SUArcCurveRef.
  SUSelectionType_Surface,      ///< Query for the selection containing only faces
                                ///< that belong to a single surface. SketchUp
                                ///< considers set of faces connected with
                                ///< soft+smooth edges to form a surface.
  SUSelectionType_SingleObject  ///< Query for a single SUDrawingElementRef being
                                ///< selected or satifying the conditions of
                                ///< SU_SELECTION_CURVE or SU_SELECTION_SURFACE.
};

/**
@brief Adds items to the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection    The selection object.
@param[in] num_elements The length of the array of drawing elements.
@param[in] elements     The array of drawing elements objects to add.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if elements is NULL
*/
SU_RESULT SUSelectionAdd(
    SUSelectionRef selection, size_t num_elements, const SUDrawingElementRef elements[]);

/**
@brief Removes items from the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection    The selection object.
@param[in] num_elements The length of the array of drawing elements.
@param[in] elements     The array of drawing elements objects to remove.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if elements is NULL
*/
SU_RESULT SUSelectionRemove(
    SUSelectionRef selection, size_t num_elements, const SUDrawingElementRef elements[]);

/**
@brief Toggle items in the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection    The selection object.
@param[in] num_elements The length of the array of drawing elements.
@param[in] elements     The array of drawing elements objects to toggle.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if elements is NULL
*/
SU_RESULT SUSelectionToggle(
    SUSelectionRef selection, size_t num_elements, const SUDrawingElementRef elements[]);

/**
@brief Clears the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection    The selection object.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
*/
SU_RESULT SUSelectionClear(SUSelectionRef selection);

/**
@brief Inverts the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection    The selection object.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
*/
SU_RESULT SUSelectionInvert(SUSelectionRef selection);

/**
@brief Gets the number of items in the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection     The selection object.
@param[out] num_elements The number of items in the selection set.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_elements is NULL
*/
SU_RESULT SUSelectionGetNumElements(SUSelectionRef selection, size_t* num_elements);

/**
@brief Fills the list with items from the selection set.
@since SketchUp 2020.2, API 8.2
@param[in] selection     The selection object.
@param[out] entity_list  The list object to be filled.
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_INVALID_OUTPUT if entity_list not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity_list is NULL
*/
SU_RESULT SUSelectionGetEntityList(SUSelectionRef selection, SUEntityListRef* entity_list);

/**
@brief Used to determine the type of selection.
@since SketchUp 2020.2, API 8.2
@param[in] selection  The selection object.
@param[in] type       The type of selection to query for.
@param[out] is_type   True if the selection is of the type queried for.
@see SUSelectionType
@related SUSelectionRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if selection is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if type is not a supported value
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_curve is NULL
*/
SU_RESULT SUSelectionIsType(SUSelectionRef selection, enum SUSelectionType type, bool* is_type);

#ifdef __cplusplus
}
#endif
#pragma pack(pop)

#endif  // SKETCHUP_MODEL_SELECTION_H_
