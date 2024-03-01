// Copyright 2014 Trimble Inc.  All rights reserved.

/**
 * @file
 * @brief Interfaces for SUShadowInfoRef.
 */
#ifndef SKETCHUP_MODEL_SHADOW_INFO_H_
#define SKETCHUP_MODEL_SHADOW_INFO_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUShadowInfoRef
@brief  Used to get and set values in a shadow info object.
@since SketchUp 2015, API 3.0
*/

/**
@brief  Gets the number of available shadow info keys.
@since SketchUp 2015, API 3.0
@param[in]  shadow_info The shadow info object.
@param[out] count       The number of keys available.
@related SUShadowInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if shadow_info is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
 */
SU_RESULT SUShadowInfoGetNumKeys(SUShadowInfoRef shadow_info, size_t* count);

/**
@brief  Retrieves keys associated with the shadow options object.
@since SketchUp 2015, API 3.0
@param[in]  shadow_info The shadow_info object.
@param[in]  len         The number of keys to retrieve.
@param[out] keys        The keys retrieved.
@param[out] count       The number of keys retrieved.
@related SUShadowInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if shadow_info is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if keys or count is NULL
- \ref SU_ERROR_INVALID_OUTPUT if any of the strings in the keys array are
  invalid.
 */
SU_RESULT SUShadowInfoGetKeys(
    SUShadowInfoRef shadow_info, size_t len, SUStringRef keys[], size_t* count);

/**
@brief  Retrieves a value from a shadow info object.
@since SketchUp 2015, API 3.0
@param[in]  shadow_info The shadow info object.
@param[in]  key         The key. Assumed to be UTF-8 encoded.
@param[out] value_out   The value retrieved.
@related SUShadowInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if shadow_info is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value_out is an invalid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if value_out is NULL
- \ref SU_ERROR_NO_DATA if the key does not exist

The list of shadow information keys is shown below.

- City (in Model Info > Geo-location > Set Manual Location...) Note that 'City' is called 'Location'
in the UI
- Country (in Model Info > Geo-location > Set Manual Location...)
- Dark (in Window > Shadows)
- DayOfYear
- DaylightSavings
- DisplayNorth (in View > Toolbars > Solar North) Note that 'Toolbar' is called 'Tool Palettes' on
Mac
- DisplayOnAllFaces (in Window > Shadows)
- DisplayOnGroundPlane (in Window > Shadows)
- DisplayShadows (in Window > Shadows)
- EdgesCastShadows (in Window > Shadows)
- Latitude (in Model Info > Geo-location > Set Manual Location...)
- Light (in Window > Shadows)
- Longitude (in Model Info > Geo-location > Set Manual Location...)
- NorthAngle (in View > Toolbars > Solar North) Note that 'Toolbar' is called 'Tool Palettes' on Mac
- ShadowTime (in Window > Shadows)
- ShadowTime_time_t (ShadowTime in Epoch time)
- SunDirection (Generated based on ShadowTime)
- SunRise (Generated based on ShadowTime)
- SunRise_time_t (SunRise in Epoch time)
- SunSet (Generated based on ShadowTime)
- SunSet_time_t (SunSet in Epoch time)
- TZOffset (in Window > Shadows)
- UseSunForAllShading (in Window > Shadows)*/
SU_RESULT SUShadowInfoGetValue(
    SUShadowInfoRef shadow_info, const char* key, SUTypedValueRef* value_out);

/**
@brief  Sets a value on a shadow info object.
@since SketchUp 2015, API 3.0
@param[in]  shadow_info The shadow info object.
@param[in]  key         The key. Assumed to be UTF-8 encoded.
@param[in]  value_in    The value used to set the shadow info option is set to.
@related SUShadowInfoRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if shadow_info or value_in is an invalid object
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_NO_DATA if the key does not exist

The list of shadow information keys is shown below.

- City (in Model Info > Geo-location > Set Manual Location...) Note that 'City' is called 'Location'
in the UI
- Country (in Model Info > Geo-location > Set Manual Location...)
- Dark (in Window > Shadows)
- DayOfYear
- DaylightSavings
- DisplayNorth (in View > Toolbars > Solar North) Note that 'Toolbar' is called 'Tool Palettes' on
Mac
- DisplayOnAllFaces (in Window > Shadows)
- DisplayOnGroundPlane (in Window > Shadows)
- DisplayShadows (in Window > Shadows)
- EdgesCastShadows (in Window > Shadows)
- Latitude (in Model Info > Geo-location > Set Manual Location...)
- Light (in Window > Shadows)
- Longitude (in Model Info > Geo-location > Set Manual Location...)
- NorthAngle (in View > Toolbars > Solar North) Note that 'Toolbar' is called 'Tool Palettes' on Mac
- ShadowTime (in Window > Shadows)
- ShadowTime_time_t (ShadowTime in Epoch time)
- SunDirection (Generated based on ShadowTime)
- SunRise (Generated based on ShadowTime)
- SunRise_time_t (SunRise in Epoch time)
- SunSet (Generated based on ShadowTime)
- SunSet_time_t (SunSet in Epoch time)
- TZOffset (in Window > Shadows)
- UseSunForAllShading (in Window > Shadows)*/
SU_RESULT SUShadowInfoSetValue(
    SUShadowInfoRef shadow_info, const char* key, SUTypedValueRef value_in);

#ifdef __cplusplus
}
#endif

#endif  // SKETCHUP_MODEL_SHADOW_INFO_H_
