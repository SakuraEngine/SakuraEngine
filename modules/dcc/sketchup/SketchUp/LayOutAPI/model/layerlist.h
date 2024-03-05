// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_LAYER_LIST_H_
#define LAYOUT_MODEL_LAYER_LIST_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOLayerListRef
@brief References an ordered, indexable list of \ref LOLayerRef objects.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new empty layer list object.
@param[out] layer_list The layer definition list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_list is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_list already refers to a valid object
*/
LO_RESULT LOLayerListCreate(LOLayerListRef* layer_list);

/**
@brief Releases a layer list object. *layer_list will be set to invalid by this
       function.
@param[in] layer_list The layer definition list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if layer_list is NULL
- \ref SU_ERROR_INVALID_INPUT if *layer_list does not refer to a valid object
*/
LO_RESULT LOLayerListRelease(LOLayerListRef* layer_list);

/**
@brief Gets the number of layer definitions in the list.
@param[in]  layer_list The layer definition list object.
@param[out] num_layers The number of layer definitions in the list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_list does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_layers is NULL
*/
LO_RESULT LOLayerListGetNumberOfLayers(LOLayerListRef layer_list, size_t* num_layers);

/**
@brief Gets the layer definition at the specified index.
@param[in] layer_list The layer definition list object.
@param[in] index      The index of the layer definition object to get.
@param[out] layer      The layer definition object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if layer_list does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range for this list
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer already refers to a valid object
*/
LO_RESULT LOLayerListGetLayerAtIndex(LOLayerListRef layer_list, size_t index, LOLayerRef* layer);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_LAYER_LIST_H_
