// Copyright 2015-2020 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_GRID_H_
#define LAYOUT_MODEL_GRID_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

#include <SketchUpAPI/color.h>

/**
@struct LOGridRef
@brief References a document's grid settings.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the major space size of the grid.
@param[in]  grid    The grid object.
@param[out] spacing The spacing of the major divisions of the grid.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if spacing is NULL
*/
LO_RESULT LOGridGetMajorSpacing(LOGridRef grid, double* spacing);

/**
@brief Sets the gap between major grid lines for the grid.
@since LayOut 2020.1, API 5.1
@param[in] grid    The grid object.
@param[in] spacing The number of minor grid lines to have between major
                   grid lines. Must be positive.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if divisions is less than zero
*/
LO_RESULT LOGridSetMajorSpacing(LOGridRef grid, double spacing);

/**
@brief Gets the number of minor divisions of the grid.
@param[in]  grid      The grid object.
@param[out] divisions The minor division count.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if divisions is NULL
*/
LO_RESULT LOGridGetMinorDivisions(LOGridRef grid, int* divisions);

/**
@brief Sets the number of minor divisions for the grid.
@since LayOut 2020.1, API 5.1
@param[in] grid      The grid object.
@param[in] divisions The number of minor grid lines to have between major
                     grid lines. Must not be negative.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if divisions is less than zero
*/
LO_RESULT LOGridSetMinorDivisions(LOGridRef grid, int divisions);

/**
@brief Gets the color for the major grid lines.
@param[in]  grid  The grid object.
@param[out] color The color of the major grid lines.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
LO_RESULT LOGridGetMajorColor(LOGridRef grid, SUColor* color);

/**
@brief Sets the color for the major grid lines.
@since LayOut 2020.1, API 5.1
@param[in] grid  The grid object.
@param[in] color The new color for the major grid lines.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetMajorColor(LOGridRef grid, SUColor color);

/**
@brief Gets the color for the minor grid lines.
@param[in]  grid  The grid object.
@param[out] color The color of the minor grid lines.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if color is NULL
*/
LO_RESULT LOGridGetMinorColor(LOGridRef grid, SUColor* color);

/**
@brief Sets the color for the minor grid lines.
@since LayOut 2020.1, API 5.1
@param[in] grid  The grid object.
@param[in] color The new color for the minor grid lines.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetMinorColor(LOGridRef grid, SUColor color);

/**
@brief Gets whether or not the grid is visible.
@param[in]  grid The grid object.
@param[out] show Whether or not the grid is visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if show is NULL
*/
LO_RESULT LOGridGetShow(LOGridRef grid, bool* show);

/**
@brief Sets whether or not the grid is visible.
@since LayOut 2020.1, API 5.1
@param[in] grid The grid object.
@param[in] show Whether or not the grid is visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetShow(LOGridRef grid, bool show);

/**
@brief Gets whether or not the major grid lines are visible.
@param[in]  grid The grid object.
@param[out] show Whether or not the major grid lines are visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if show is NULL
*/
LO_RESULT LOGridGetShowMajor(LOGridRef grid, bool* show);

/**
@brief Sets whether or not the major grid lines are visible.
@since LayOut 2020.1, API 5.1
@param[in] grid The grid object.
@param[in] show Whether or not the major grid lines are visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetShowMajor(LOGridRef grid, bool show);

/**
@brief Gets whether or not the minor grid lines are visible.
@param[in]  grid The grid object.
@param[out] show Whether or not the minor grid lines are visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if show is NULL
*/
LO_RESULT LOGridGetShowMinor(LOGridRef grid, bool* show);

/**
@brief Sets whether or not the minor grid lines are visible.
@since LayOut 2020.1, API 5.1
@param[in] grid The grid object.
@param[in] show Whether or not the minor grid lines are visible.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetShowMinor(LOGridRef grid, bool show);

/**
@brief Gets whether or not the grid will be printed.
@param[in]  grid  The grid object.
@param[out] print Whether or not the grid will be printed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if print is NULL
*/
LO_RESULT LOGridGetPrint(LOGridRef grid, bool* print);

/**
@brief Sets whether or not the grid is will be printed.
@since LayOut 2020.1, API 5.1
@param[in] grid  The grid object.
@param[in] print Whether or not the grid will be printed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetPrint(LOGridRef grid, bool print);

/**
@brief Gets whether or not the grid is drawn on top of entities.
@since LayOut 2020.1, API 5.1
@param[in]  grid     The grid object.
@param[out] in_front Whether or not the grid is on top of entities.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if in_front is NULL
*/
LO_RESULT LOGridGetInFront(LOGridRef grid, bool* in_front);

/**
@brief Sets whether or not the grid is drawn on top of entities.
@since LayOut 2020.1, API 5.1
@param[in] grid     The grid object.
@param[in] in_front Whether or not the grid is on top of entities.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetInFront(LOGridRef grid, bool in_front);

/**
@brief Gets whether or not the grid is clipped to the paper margins.
@since LayOut 2020.1, API 5.1
@param[in]  grid The grid object.
@param[out] clip Whether or not the grid is on top of entities.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if clipped is NULL
*/
LO_RESULT LOGridGetClipToMargins(LOGridRef grid, bool* clip);

/**
@brief Sets whether or not the grid is clipped to the paper margins.
@since LayOut 2020.1, API 5.1
@param[in] grid The grid object.
@param[in] clip Whether or not the grid is clipped to the paper margins.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if grid does not refer to a valid object
*/
LO_RESULT LOGridSetClipToMargins(LOGridRef grid, bool clip);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_GRID_H_
