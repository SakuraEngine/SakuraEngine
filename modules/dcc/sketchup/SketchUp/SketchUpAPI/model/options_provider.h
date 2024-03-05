// Copyright 2013-2019 Trimble Inc.  All Rights Reserved

/**
 * @file
 * @brief Interfaces for SUOptionsProviderRef.
 */
#ifndef SKETCHUP_MODEL_OPTIONS_PROVIDER_H_
#define SKETCHUP_MODEL_OPTIONS_PROVIDER_H_

#include <SketchUpAPI/common.h>
#include <SketchUpAPI/unicodestring.h>
#include <SketchUpAPI/model/defs.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@struct SUOptionsProviderRef
@brief  Provides ability to get and set options on an options provider object.
        Available options providers are: PageOptions, SlideshowOptions,
        UnitsOptions and PrintOptions.
*/

/**
@brief  Gets the number of available option keys.
@param[in]  options_provider The options provider object.
@param[out] count            The number of keys available.
@related SUOptionsProviderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_provider is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if count is NULL
 */
SU_RESULT SUOptionsProviderGetNumKeys(SUOptionsProviderRef options_provider, size_t* count);

/**
@brief  Retrieves options providers associated with the options manager.
@param[in]  options_provider The options provider object.
@param[in]  len              The number of keys to retrieve.
@param[out] keys             The keys retrieved.
@param[out] count            The number of keys retrieved.
@related SUOptionsProviderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_provider is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if keys or count is NULL
- \ref SU_ERROR_INVALID_OUTPUT if any of the strings in the keys array are
  invalid.
 */
SU_RESULT SUOptionsProviderGetKeys(
    SUOptionsProviderRef options_provider, size_t len, SUStringRef keys[], size_t* count);

// clang-format off
/**
@brief  Gets the value of the given option.
@param[in]  options_provider The options provider object.
@param[in]  key              The key that indicates which option to get.
@param[out] value            The value to get the current option setting.
@related SUOptionsProviderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_provider is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if value is NULL
- \ref SU_ERROR_INVALID_OUTPUT if value is invalid

The breakdown of options and value types for each options provider is shown
in the table below.

Options Provider | Option               | Value Type              | Meaning
---------------- | -------------------- | ----------------------- | -------
NamedOptions     | &nbsp;               | &nbsp;                  | (SketchUp 2022.0, API 10.0) Removed from API
PageOptions      | &nbsp;               | &nbsp;                  | Options for the Scene
&nbsp;           | ShowTransition       | SUTypedValueType_Bool   | Show scene transitions
&nbsp;           | TransitionTime       | SUTypedValueType_Double | Number of seconds between each scene transition
SlideshowOptions | &nbsp;               | &nbsp;                  | Options for the slideshow
&nbsp;           | LoopSlideshow        | SUTypedValueType_Bool   | Causes the slideshow to loop
&nbsp;           | SlideTime            | SUTypedValueType_Double | Number of seconds that each slide is shown
UnitsOptions     | &nbsp;               | &nbsp;                  | Options for units display in the model
&nbsp;           | LengthPrecision      | SUTypedValueType_Int32  | Number of decimal places of precision shown for length
&nbsp;           | LengthFormat         | SUTypedValueType_Int32  | Default units format for the model
&nbsp;           | LengthUnit           | SUTypedValueType_Int32  | Units format for the model
&nbsp;           | LengthSnapEnabled    | SUTypedValueType_Bool   | Indicates whether length snapping is enabled
&nbsp;           | LengthSnapLength     | SUTypedValueType_Double | Controls the snapping length size increment
&nbsp;           | AreaPrecision        | SUTypedValueType_Int32  | (SketchUp 2020.0, API Version 8.0) Number of decimal places of precision shown for area units
&nbsp;           | AreaUnit             | SUTypedValueType_Int32  | (SketchUp 2019.2, API Version 7.1) Area units format for the model
&nbsp;           | VolumePrecision      | SUTypedValueType_Int32  | (SketchUp 2020.0, API Version 8.0) Number of decimal places of precision shown for volume units
&nbsp;           | VolumeUnit           | SUTypedValueType_Int32  | (SketchUp 2019.2, API Version 7.1) Volume units format for the model
&nbsp;           | AnglePrecision       | SUTypedValueType_Int32  | Number of decimal places of precision shown for angles
&nbsp;           | AngleSnapEnabled     | SUTypedValueType_Bool   | Indicates whether angle snapping is enabled
&nbsp;           | SnapAngle            | SUTypedValueType_Double | Controls the angle snapping size increment
&nbsp;           | SuppressUnitsDisplay | SUTypedValueType_Bool   | Display the units format if LengthFormat is Decimal or Fractional
&nbsp;           | ForceInchDisplay     | SUTypedValueType_Bool   | Force displaying 0" if LengthFormat is Architectural

Some of the options map to enumerated values, as shown in the table below.

Option       | Value | Meaning
------------ | ----- | -------
LengthFormat | 0:    | Decimal
&nbsp;       | 1:    | Architectural
&nbsp;       | 2:    | Engineering
&nbsp;       | 3:    | Fractional
LengthUnit   | 0:    | Inches
&nbsp;       | 1:    | Feet
&nbsp;       | 2:    | Millimeter
&nbsp;       | 3:    | Centimeter
&nbsp;       | 4:    | Meter
&nbsp;       | 5:    | Yard
AreaUnit     | 0:    | Square Inches
&nbsp;       | 1:    | Square Feet
&nbsp;       | 2:    | Square Millimeter
&nbsp;       | 3:    | Square Centimeter
&nbsp;       | 4:    | Square Meter
&nbsp;       | 5:    | Square Yard
VolumeUnit   | 0:    | Cubic Inches
&nbsp;       | 1:    | Cubic Feet
&nbsp;       | 2:    | Cubic Millimeter
&nbsp;       | 3:    | Cubic Centimeter
&nbsp;       | 4:    | Cubic Meter
&nbsp;       | 5:    | Cubic Yard
&nbsp;       | 6:    | Liter
&nbsp;       | 7:    | US Gallon

Note that LengthUnit will be overridden by LengthFormat if LengthFormat is not
set to Decimal. Architectural defaults to inches, Engineering defaults to feet,
and Fractional defaults to inches.
 */
 // clang-format
SU_RESULT SUOptionsProviderGetValue(
    SUOptionsProviderRef options_provider, const char* key, SUTypedValueRef* value);

/**
@brief  Sets the value of the given option.
@param[in] options_provider The options provider object.
@param[in] key              The key that indicates which option to set.
@param[in] value            The value to set the option to.
@related SUOptionsProviderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_provider or value is not valid
- \ref SU_ERROR_NULL_POINTER_INPUT if key is NULL
 */
SU_RESULT SUOptionsProviderSetValue(
    SUOptionsProviderRef options_provider, const char* key, SUTypedValueRef value);

/**
@brief Retrieves the name of the options provider.
@since SketchUp 2016, API 4.0
@param[in] options_provider The options provider object.
@param[out] name            The name retrieved.
@related SUOptionsProviderRef
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if options_provider or value is not valid
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
- \ref SU_ERROR_INVALID_OUTPUT if name does not point to a valid \ref
SUStringRef object
*/
SU_RESULT SUOptionsProviderGetName(SUOptionsProviderRef options_provider, SUStringRef* name);

#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // SKETCHUP_MODEL_OPTIONS_PROVIDER_H_
