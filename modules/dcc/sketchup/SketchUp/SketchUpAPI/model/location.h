// Copyright 2013 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SULocationRef.
 */
#ifndef SKETCHUP_MODEL_LOCATION_H_
#define SKETCHUP_MODEL_LOCATION_H_

#include <SketchUpAPI/model/defs.h>

/**
@struct SULocationRef
@brief References a type that contains location information of a model
   (e.g. latitude, longitude).
*/

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Retrieves the latitude and longitude of a location object.
@param[in]  location  The location object.
@param[out] latitude  The latitude value retrieved.
@param[out] longitude The longitude value retrieved.
@related SULocationRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if location is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if latitude or longitude is NULL
*/
SU_RESULT SULocationGetLatLong(SULocationRef location, double* latitude, double* longitude);

/**
@brief Assigns the latitude and longitude values of a location object.
@param[in] location  The location object.
@param[in] latitude  The latitude value to assign.
@param[in] longitude The longitude value to assign.
@related SULocationRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if location is not a valid object or if the
  latitude is out of range [-90, 90] or if longitude is out of range
  [-180, 180]
*/
SU_RESULT SULocationSetLatLong(SULocationRef location, double latitude, double longitude);

/**
@brief Assigns the north angle values of a location object.
@param[in] location    The location object.
@param[in] north_angle The north angle value to assign.
@related SULocationRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if location is not a valid object or if north
  angle is out of range [0, 360]
*/
SU_RESULT SULocationSetNorthAngle(SULocationRef location, double north_angle);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // SKETCHUP_MODEL_LOCATION_H_
