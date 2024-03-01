// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_ENTITYITERATOR_H_
#define LAYOUT_MODEL_ENTITYITERATOR_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOEntityIteratorRef
@brief References an iterator for \ref LOEntityRef objects.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Releases an entity iterator object. *entity_iterator will be set to
       invalid by this function.
@param[in] entity_iterator The entity iterator object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if entity_iterator is NULL
- \ref SU_ERROR_INVALID_INPUT if *entity_iterator does not refer to a valid
object
*/
LO_RESULT LOEntityIteratorRelease(LOEntityIteratorRef* entity_iterator);

/**
@brief Gets a reference to the current entity and increment the iterator.
       entity will be set to an invalid object upon reaching the end of the
       iterator.
@param[in]  entity_iterator The entity iterator object.
@param[out] entity          The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if entity_iterator does not refer to a valid
  object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity does not refer to a valid object
*/
LO_RESULT LOEntityIteratorNext(LOEntityIteratorRef entity_iterator, LOEntityRef* entity);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_ENTITYITERATOR_H_
