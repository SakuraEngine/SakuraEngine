// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_PAGE_LIST_H_
#define LAYOUT_MODEL_PAGE_LIST_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOPageListRef
@brief References an ordered, indexable list of \ref LOPageRef objects.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new empty page list object.
@param[out] page_list An empty page list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page_list is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page_list already refers to a valid object
*/
LO_RESULT LOPageListCreate(LOPageListRef* page_list);

/**
@brief Releases a page list object. *page_list will be set to invalid by this
       function.
@param[in] page_list The page list object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if page_list does not refer to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if *page_list does not refer to a valid object
*/
LO_RESULT LOPageListRelease(LOPageListRef* page_list);

/**
@brief Gets the number of pages in this list.
@param[in]  page_list The page list object.
@param[out] num_pages The number of page objects in the list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page_list does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_pages is NULL
*/
LO_RESULT LOPageListGetNumberOfPages(LOPageListRef page_list, size_t* num_pages);

/**
@brief Gets the page at the specified index.
@param[in]  page_list The page list object.
@param[in]  index     The index of the page to get.
@param[out] page      The page object at index in the list.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page_list does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range for this list
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page already refers to a valid object
*/
LO_RESULT LOPageListGetPageAtIndex(LOPageListRef page_list, size_t index, LOPageRef* page);

/**
@brief Adds a page at the end of the list.
@param[in] page_list The page list object.
@param[in] page      The page object to add.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if page_list does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
*/
LO_RESULT LOPageListAddPage(LOPageListRef page_list, LOPageRef page);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_PAGE_LIST_H_
