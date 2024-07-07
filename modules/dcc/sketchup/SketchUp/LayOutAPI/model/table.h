// Copyright 2016 Trimble Navigation Ltd., All rights reserved.

#ifndef LAYOUT_MODEL_TABLE_H_
#define LAYOUT_MODEL_TABLE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
 @struct LOTableRef
 @brief References a table. A table is a series of rows and columns that holds
        data.
 */

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@enum LOTableCellRotation
@brief Defines the rotation angle for table cells.
*/
typedef enum {
  LOTableCellRotation_0 = 0,  ///< No rotation.
  LOTableCellRotation_90,     ///< Rotated 90 degrees counter-clockwise.
  LOTableCellRotation_180,    ///< Rotated 180 degrees (upside-down).
  LOTableCellRotation_270     ///< Rotated 270 degrees counter-clockwise.
} LOTableCellRotation;

/**
@brief Creates a table with a specified size, and a specified number of rows and
columns.
@param[out] table   The table object.
@param[in]  bounds  The starting dimensions of the table.
@param[in]  rows    The number of rows.
@param[in]  columns The number of columns.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if table is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *table already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
- \ref SU_ERROR_OUT_OF_RANGE if rows is less than 1
- \ref SU_ERROR_OUT_OF_RANGE if columns is less than 1
*/
LO_RESULT LOTableCreate(
    LOTableRef* table, const LOAxisAlignedRect2D* bounds, size_t rows, size_t columns);
/**
@brief Gets a table from a given entity.
@since LayOut 2017, API 2.0
@param[in] entity The entity object.
@return
- The converted \ref LOTableRef if the downcast operation succeeds
- If not, the returned reference will be invalid
*/
LO_EXPORT LOTableRef LOTableFromEntity(LOEntityRef entity);

/**
@brief Converts from a \ref LOTableRef to a \ref LOEntityRef.
This is essentially an upcast operation.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@return
- The converted \ref LOEntityRef if table is a valid object
- If not, the returned reference will be invalid
*/
LO_EXPORT LOEntityRef LOTableToEntity(LOTableRef table);

/**
@brief Creates the entities that represent the tabel in its exploded form and
       adds them to a \ref LOEntityListRef. It is NOT necessary to explicitly
       release these entities, since \ref LOEntityListRef itself adds a
       reference to the entities and will release them when they are removed
       from the list or when the list is released.
@since LayOut 2017, API 2.0
@param[in] table       The table object.
@param[in] entity_list The entity list object to add the new entities to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity_list does not refer to a valid object
*/
LO_RESULT LOTableGetExplodedEntities(LOTableRef table, LOEntityListRef entity_list);

/**
@brief Releases a table object. The object will be invalidated if releasing the
       last reference.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if table is NULL
- \ref SU_ERROR_INVALID_INPUT if *table does not refer to a valid object
*/
LO_RESULT LOTableRelease(LOTableRef* table);

/**
@brief Adds a reference to a table object.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
*/
LO_RESULT LOTableAddReference(LOTableRef table);

/**
@brief Gets the number of rows and columns in a table.
@since LayOut 2017, API 2.0
@param[in]  table   The table object.
@param[out] rows    The number of rows.
@param[out] columns The number of columns.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rows or columns are NULL
*/
LO_RESULT LOTableGetDimensions(LOTableRef table, size_t* rows, size_t* columns);

/**
@brief Gets the height of a row, specified by index, in a table.
@since LayOut 2017, API 2.0
@param[in]  table  The table object.
@param[in]  index  The index of the row.
@param[out] height The height of the row.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if height is NULL
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
*/
LO_RESULT LOTableGetRowHeight(LOTableRef table, size_t index, double* height);

/**
@brief Sets the height of a row, specified by index, in a table. There is a
       minimum allowable row height specified by kMinimumRowHeight.
@since LayOut 2017, API 2.0
@param[in]  table  The table object.
@param[in]  index  The index of the row.
@param[out] height The height of the row.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if height is not a valid row height
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetRowHeight(LOTableRef table, size_t index, double height);

/**
@brief Gets the width of a column, specified by index, in a table.
@since LayOut 2017, API 2.0
@param[in]  table The table object.
@param[in]  index The index of the row.
@param[out] width The height of the row.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if width is NULL
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
*/
LO_RESULT LOTableGetColumnWidth(LOTableRef table, size_t index, double* width);
/**
@brief Sets the width of a column, specified by index, in a table. There is a
       minimum allowable column width specified by kMinimumColumnWidth.
@since LayOut 2017, API 2.0
@param[in]  table The table object.
@param[in]  index The index of the column.
@param[out] width The width of the column.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if width is is not a valid column width
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetColumnWidth(LOTableRef table, size_t index, double width);

/**
@brief Inserts a row at the specified index.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@param[in] index The index of the row to be inserted at.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableInsertRow(LOTableRef table, size_t index);

/**
@brief Removes a row at the specified index.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@param[in] index The index of the row to be removed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableRemoveRow(LOTableRef table, size_t index);

/**
@brief Inserts a column at the specified index.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@param[in] index The index of the column to be inserted at.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableInsertColumn(LOTableRef table, size_t index);

/**
@brief Removes a column at the specified index.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@param[in] index The index of the column to be removed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index for table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableRemoveColumn(LOTableRef table, size_t index);

/**
@brief Creates a copy of the formatted text entity for a table cell at the
       specified row and column. Currently, this will always succeed. However,
       future versions of LayOut may support other types of entities for table
       cells, so you should not assume that this will succeed. If the specified
       row and column is within a merged cell, then a copy of the merged cell's
       text entity will be created.
@since LayOut 2017, API 2.0
@param[in]  table  The table object.
@param[in]  row    The cell's row.
@param[in]  column The cell's column.
@param[out] text   A copy of the cell's text entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row or column are out of range for the table
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity already refers to a valid object
- \ref SU_ERROR_NO_DATA if the cell entity is not a formatted text entity
*/
LO_RESULT LOTableCreateCellTextCopy(
    LOTableRef table, size_t row, size_t column, LOFormattedTextRef* text);

/**
@brief Sets the text entity of a table cell from a \ref LOFormattedTextRef
       object. Only the text content and fill style settings will be kept. The
       bounds and other style settings are controlled by the table. If the
       specified row and column is within a merged cell, then the merged cell
       itself will be affected.
@since LayOut 2017, API 2.0
@param[in] table  The table object.
@param[in] row    The cell's row.
@param[in] column The cell's column.
@param[in] text   The text object representing the table cell.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row or column are out of range for the table
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetCellText(LOTableRef table, size_t row, size_t column, LOFormattedTextRef text);

/**
@brief Gets the style of a table's border.
@since LayOut 2018, API 3.0
@param[in]  table The table object.
@param[out] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
*/
LO_RESULT LOTableGetBorderStyle(LOTableRef table, LOStyleRef style);

/**
@brief Sets the style of a table's border. Only the stroke style setting can be
       set via this method. Other style settings are controlled by the table.
@since LayOut 2018, API 3.0
@param[in] table The table object.
@param[in] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetBorderStyle(LOTableRef table, LOStyleRef style);

/**
@brief Gets the style of a table's inner row edge. The specified row must be
       in the range of 0 to the number of rows minus two.
@since LayOut 2017, API 2.0
@param[in]  table The table object.
@param[in]  row   The row whose edge style to get.
@param[out] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row is out of range for the table
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
*/
LO_RESULT LOTableGetRowEdgeStyle(LOTableRef table, size_t row, LOStyleRef style);

/**
@brief Sets the style of a table's inner row edge. The specified row must be
       in the range of 0 to the number of rows minus two. Only the stroke style
       setting can be set via this method. Other style settings are controlled
       by the table.
@since LayOut 2017, API 2.0
@param[in] table The table object.
@param[in] row   The row whose edge style to set.
@param[in] style The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row is out of range for the table
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetRowEdgeStyle(LOTableRef table, size_t row, LOStyleRef style);

/**
@brief Gets the style of a table's inner column edge. The specified column must
       be in the range of 0 to the number of columns minus two.
@since LayOut 2017, API 2.0
@param[in]  table  The table object.
@param[in]  column The column whose edge style to get.
@param[out] style  The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if column is out of range for the table
- \ref SU_ERROR_INVALID_OUTPUT if style does not refer to a valid object
*/
LO_RESULT LOTableGetColumnEdgeStyle(LOTableRef table, size_t column, LOStyleRef style);

/**
@brief Sets the style of a table's inner column edge. The specified column must
       be in the range of 0 to the number of rows minus two. Only the stroke
       style setting can be set via this method. Other style settings are
       controlled by the table.
@since LayOut 2017, API 2.0
@param[in] table  The table object.
@param[in] column The column whose edge style to set.
@param[in] style  The style object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if column is out of range for the table
- \ref SU_ERROR_INVALID_INPUT if style does not refer to a valid object
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
*/
LO_RESULT LOTableSetColumnEdgeStyle(LOTableRef table, size_t column, LOStyleRef style);

/**
@brief Gets the row and column span of a table cell. If the values returned by
       both row_span and column_span are equal to 1, then it is a normal,
       non-merged cell. If either of these values are greater than 1, then it
       is a merged cell. If these values are both 0, then it is an unused cell
       that resides within the inner portion of another merged cell.
@since LayOut 2017, API 2.0
@param[in]  table       The table object.
@param[in]  row         The row index.
@param[in]  column      The column index.
@param[out] row_span    The number of rows that this cell spans.
@param[out] column_span The number of columns that this cell spans.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row is out of range for the table
- \ref SU_ERROR_OUT_OF_RANGE if column is out of range for the table
- \ref SU_ERROR_NULL_POINTER_OUTPUT if row_span is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if column_span is NULL
*/
LO_RESULT LOTableGetCellSpan(
    LOTableRef table, size_t row, size_t column, size_t* row_span, size_t* column_span);

/**
@brief Merge a range of cells within a table. Only cells which are not already
       merged can be merged.
@since LayOut 2017, API 2.0
@param[in]  table        The table object.
@param[in]  start_row    The start row index.
@param[in]  start_column The start column index.
@param[out] end_row      The end row index.
@param[out] end_column   The end column index.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if any of the row or column indices are out of
       range for the table
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
- \ref SU_ERROR_UNSUPPORTED if the specified range of cells only spans a single
       cell
- \ref SU_ERROR_UNSUPPORTED if the specified range of cells contains a cell that
       is already merged
*/
LO_RESULT LOTableMergeCells(
    LOTableRef table, size_t start_row, size_t start_column, size_t end_row, size_t end_column);

/**
@brief Gets the rotation of a table cell.
@since LayOut 2017, API 2.0
@param[in]  table    The table object.
@param[in]  row      The cell's row.
@param[in]  column   The cell's column.
@param[out] rotation The cell's rotation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row is out of range for the table
- \ref SU_ERROR_OUT_OF_RANGE if column is out of range for the table
- \ref SU_ERROR_NULL_POINTER_OUTPUT if rotation is NULL
- \ref SU_ERROR_NO_DATA if the specific cell is invalid due to residing within
                        a merged cell
*/
LO_RESULT LOTableGetCellRotation(
    LOTableRef table, size_t row, size_t column, LOTableCellRotation* rotation);

/**
@brief Sets the rotation of a table cell.
@since LayOut 2017, API 2.0
@param[in]  table    The table object.
@param[in]  row      The cell's row.
@param[in]  column   The cell's column.
@param[out] rotation The cell's rotation.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if table does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if row is out of range for the table
- \ref SU_ERROR_OUT_OF_RANGE if column is out of range for the table
- \ref SU_ERROR_OUT_OF_RANGE if rotation is invalid
- \ref SU_ERROR_LAYER_LOCKED if the table is on a locked layer
- \ref SU_ERROR_ENTITY_LOCKED if table is locked
- \ref SU_ERROR_UNSUPPORTED  if the specific cell is invalid due to residing
                             within a merged cell
*/
LO_RESULT LOTableSetCellRotation(
    LOTableRef table, size_t row, size_t column, LOTableCellRotation rotation);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_TABLE_H_
