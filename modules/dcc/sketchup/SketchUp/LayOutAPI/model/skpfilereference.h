// Copyright 2017 Trimble Inc., All rights reserved.

#ifndef LAYOUT_MODEL_SKPFILEREFERENCE_H_
#define LAYOUT_MODEL_SKPFILEREFERENCE_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>
#include <SketchUpAPI/model/defs.h>

/**
 @struct LOSkpFileRef
 @since LayOut 2018, API 3.0
 @brief References a SketchUp file reference object.
 */
DEFINE_SU_TYPE(LOSkpFileRef)

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a SketchUp file reference object with a Skp model.
       This function takes ownership of the given model. Upon success, user
       should not release the model, or create another LOSkpFileRef with
       the same model. The model is valid and mutable till the LOSkpFileRef is
       released.
       Do not use a model obtained by a 'Get' function such as the
       \ref LOSketchUpModelGetModel() as the ownership cannot be transfered
       to the file_ref.
@since LayOut 2018, API 3.0
@param[out] file_ref The file reference object to be created.
@param[in]  model    The SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if file_ref is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *file_ref refers to a valid object
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
*/
LO_RESULT LOSkpFileReferenceCreate(LOSkpFileRef* file_ref, SUModelRef model);

/**
@brief Releases a SketchUp file reference object.
@since LayOut 2018, API 3.0
@param[in] file_ref The SketchUp file reference object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if file_ref is NULL
- \ref SU_ERROR_INVALID_INPUT if *file_ref does not refer to a valid object
*/
LO_RESULT LOSkpFileReferenceRelease(LOSkpFileRef* file_ref);

/**
@brief Creates a new SketchUp model object with the specified bounds and
       file reference.
       This function increases reference count on the given file reference.
@since LayOut 2018, API 3.0
@param[out] model    The model entity to be created.
@param[in]  file_ref The SketchUp file reference object.
@param[in]  bounds   The starting dimensions of the model entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if model is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *model already refers to a valid object
- \ref SU_ERROR_INVALID_INPUT if file_ref is invalid
- \ref SU_ERROR_NULL_POINTER_INPUT if bounds is NULL
- \ref SU_ERROR_OUT_OF_RANGE if bounds has a width or height of zero
*/
LO_RESULT LOSketchUpModelCreateWithSkpFileRef(
    LOSketchUpModelRef* model, LOSkpFileRef file_ref, const LOAxisAlignedRect2D* bounds);

/**
@brief Applies a clip mask to all entities inside a group. Entities outside
       the clip mask will be removed from the document. Entities that cross
       the clip mask boundary will be removed but may be replaced by new
       entities. The clip_mask entity must be a \ref LORectangleRef,
       \ref LOEllipseRef, or \ref LOPathRef.
@since LayOut 2018, API 3.0
@param[in]  group     The group to apply the clipmask to.
@param[in]  clip_mask The entity to use as a clip mask.
@return
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_ENTITY_LOCKED if group is locked
- \ref SU_ERROR_LAYER_LOCKED if group contains entities on locked layers
- \ref SU_ERROR_GENERIC if clipping algorithm fails
*/
LO_RESULT LOGroupClipChildren(LOGroupRef group, LOEntityRef clip_mask);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_SKPFILEREFERENCE_H_
