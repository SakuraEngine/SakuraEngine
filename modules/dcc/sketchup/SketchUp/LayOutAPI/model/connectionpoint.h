// Copyright 2015 Trimble Navigation Ltd. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_CONNECTIONPOINT_H_
#define LAYOUT_MODEL_CONNECTIONPOINT_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/geometry/geometry.h>
#include <LayOutAPI/model/defs.h>

/**
@struct LOConnectionPointRef
@brief References a connection point. A connection point defines a target
       point to which a label or dimension can connect.
*/

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Creates a new connection point object for the given entity at the given
       2D point.
@param[out] connection_point The connection point object.
@param[in]  entity           The entity object.
@param[in]  point            The position of the connection point.
@param[in]  aperture         The search radius to use when looking for geometry
                             to connect to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if connection_point is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *connection_point already refers to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if aperture is less than or equal to 0
- \ref SU_ERROR_NULL_POINTER_INPUT if point is NULL
- \ref SU_ERROR_GENERIC if the entity is not in a document
- \ref SU_ERROR_NO_DATA if no connection point could be made
*/
LO_RESULT LOConnectionPointCreate(
    LOConnectionPointRef* connection_point, LOEntityRef entity, const LOPoint2D* point,
    double aperture);

/**
@brief Creates a new connection point object for the given SketchUp model entity
       at the given 3D point in model space. Note that connection points created
       via this function will have no persistent ID assigned to them, resulting
       in a connection point that does not update when geometry is modified.
@param[out] connection_point The connection point object.
@param[in]  model            The SketchUp model object.
@param[in]  point3d          The 3D position of the connection point in model
                             space.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if connection_point is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *connection_point already refers to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if point3d is NULL
- \ref SU_ERROR_GENERIC if the model is not in a document
*/
LO_RESULT LOConnectionPointCreateFromPoint3D(
    LOConnectionPointRef* connection_point, LOSketchUpModelRef model, const LOPoint3D* point3d);

/**
@brief Creates a new connection point object for the given SketchUp model entity
       at the given 3D point in model space. Note that connection points created
       via this function have a persistent ID assigned to them, resulting in a
       connection point that updates when the geometry is modified.
@note  Starting in LayOut 2020.1, API 5.1, SU_ERROR_NO_DATA will be returned if
       persistent_id isn't valid for the given model.
@since LO 2017, API 2.0
@param[out] connection_point The connection point object.
@param[in]  model            The SketchUp model object.
@param[in]  point3d          The 3D position of the connection point in model.
@param[in]  persistent_id    The PID of the entity.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if connection_point is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *connection_point already refers to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if model does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if point3d or persistent_id is NULL
- \ref SU_ERROR_GENERIC if the model is not in a document
- \ref SU_ERROR_NO_DATA if persistent_id does not refer to a valid path
*/
LO_RESULT LOConnectionPointCreateFromPID(
    LOConnectionPointRef* connection_point, LOSketchUpModelRef model, const LOPoint3D* point3d,
    const char* persistent_id);

/**
@brief Releases a connection point object. The object will be invalidated if
       releasing the last reference.
@param[in] connection_point The connection point object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if connection_point is NULL
- \ref SU_ERROR_INVALID_INPUT if *connection_point does not refer to a valid
  object
*/
LO_RESULT LOConnectionPointRelease(LOConnectionPointRef* connection_point);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_CONNECTIONPOINT_H_
