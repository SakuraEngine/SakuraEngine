// Copyright 2015 Trimble Inc., All rights reserved.
// This file is intended for public distribution.

/**
 * @file
 * @brief Generic interfaces for the API types.
 */
#ifndef SKETCHUP_COMMON_H_
#define SKETCHUP_COMMON_H_

/**
@enum SUResult
@brief Defines return values used by most API functions.
*/
enum SUResult {
  SU_ERROR_NONE = 0,  ///< Indicates success.

  SU_ERROR_NULL_POINTER_INPUT,  ///< A pointer for a required input was NULL.

  SU_ERROR_INVALID_INPUT,  ///< An API object input to the function was not
                           ///< created properly.

  SU_ERROR_NULL_POINTER_OUTPUT,  ///< A pointer for a required output was NULL.

  SU_ERROR_INVALID_OUTPUT,  ///< An API object to be written with output from the
                            ///< function was not created properly.

  SU_ERROR_OVERWRITE_VALID,  ///< Indicates that an input object reference
                             ///< already references an object where it was
                             ///< expected to be \ref SU_INVALID.

  SU_ERROR_GENERIC,  ///< Indicates an unspecified error.

  SU_ERROR_SERIALIZATION,  ///< Indicate an error occurred during loading or
                           ///< saving of a file.

  SU_ERROR_OUT_OF_RANGE,  ///< An input contained a value that was outside the
                          ///< range of allowed values.

  SU_ERROR_NO_DATA,  ///< The requested operation has no data to return to the
                     ///< user. This usually occurs when a request is made for
                     ///< data that is only available conditionally.

  SU_ERROR_INSUFFICIENT_SIZE,  ///< Indicates that the size of an output
                               ///< parameter is insufficient.

  SU_ERROR_UNKNOWN_EXCEPTION,  ///< An unknown exception occurred.

  SU_ERROR_MODEL_INVALID,  ///< The model requested is invalid and cannot be loaded.

  SU_ERROR_MODEL_VERSION,  ///< The model cannot be loaded or saved due to an
                           ///< invalid version

  SU_ERROR_LAYER_LOCKED,  ///< The layer that is being modified is locked.

  SU_ERROR_DUPLICATE,        ///< The user requested an operation that would result
                             ///< in duplicate data.
  SU_ERROR_PARTIAL_SUCCESS,  ///< The requested operation was not fully
                             ///< completed but it returned an intermediate
                             ///< successful result.

  SU_ERROR_UNSUPPORTED,  ///< The requested operation is not supported

  SU_ERROR_INVALID_ARGUMENT,  ///< An argument contains invalid information

  SU_ERROR_ENTITY_LOCKED,  ///< The entity being modified is locked

  SU_ERROR_INVALID_OPERATION,  ///< The requested operation is invalid.
};

// Define a platform-independent UTF16 type.
#if defined(__APPLE__) || defined(__LINUX__)
  #ifndef FOUNDATION_IMPORT
typedef unsigned short unichar;
  #endif  // FOUNDATION_IMPORT
#else     // WIN32
  #include <wtypes.h>
typedef wchar_t unichar;  ///< A platform-independent UTF16 type.
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS

  #define SU_EXPORT
  #if defined WIN32 && !defined GSLAPI_NO_EXPORTS
    #undef SU_EXPORT
    #ifdef GSLAPI_API_EXPORTS
      #define SU_EXPORT __declspec(dllexport)
    #else
      #define SU_EXPORT __declspec(dllimport)
    #endif  // GSLAPI_API_EXPORTS
  #endif    // WINDOWS

  #if defined __APPLE__
    #undef SU_EXPORT
    #ifdef GSLAPI_API_EXPORTS
      #define SU_EXPORT __attribute__((visibility("default")))
    #else
      #define SU_EXPORT __attribute__((visibility("hidden")))
    #endif
  #endif  // #if defined __APPLE__

  #define DEFINE_SU_TYPE(TYPENAME) \
    typedef struct {               \
      void* ptr;                   \
    } TYPENAME;

  // #define SU_RESULT SU_EXPORT enum SUResult

#endif  // DOXYGEN_SHOULD_SKIP_THIS

/**
@brief Macro shortcut for enum \ref SUResult.
*/
#define SU_RESULT SU_EXPORT enum SUResult

/**
@brief Use this macro to initialize new reference variables.
       e.g. \ref SUStringRef str = SU_INVALID;
*/
#define SU_INVALID \
  { 0 }

/**
@brief Use this macro to test for valid SU variables.
       e.g. if (SUIsValid(result)) return true;
*/
#define SUIsValid(VARIABLE) ((VARIABLE).ptr != 0)

/**
@brief Use this macro to test for invalid SU variables.
       e.g. if (SUIsInvalid(result)) return false;
*/
#define SUIsInvalid(VARIABLE) ((VARIABLE).ptr == 0)

/**
@brief Use this macro to set a reference invalid.
*/
#define SUSetInvalid(VARIABLE) (VARIABLE).ptr = 0

/**
@brief Use this macro to check if two references are equal.
*/
#define SUAreEqual(VARIABLE1, VARIABLE2) ((VARIABLE1).ptr == (VARIABLE2).ptr)

#include <stddef.h>  // for size_t

#if !defined(__STDC_HOSTED__) || (__STDC_HOSTED__ == 0)
  // The host compiler does not implement C99
  #ifdef WIN32
typedef __int64 int64_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef unsigned __int32 uint32_t;
  #else
    #error Unsupported compiler!
  #endif
#else
  #include <stdint.h>
#endif  // #if !defined(__STDC_HOSTED__) || (__STDC_HOSTED__ == 0)

/**
@brief This macro is used to indicate if functions are intended to be
       deprecated.  If you would like to hide the deprecation warnings simply
       define SU_SUPPRESS_DEPRECATION_WARNINGS
*/
#ifndef SU_SUPPRESS_DEPRECATION_WARNINGS
  #ifdef WIN32
    #define SU_DEPRECATED_FUNCTION(version_num) \
      __declspec(deprecated("first deprecated in " version_num))
  #else
    #define SU_DEPRECATED_FUNCTION(version_num) \
      __attribute__((deprecated("first deprecated in " version_num)))
  #endif
#else
  #define SU_DEPRECATED_FUNCTION(version_num)
#endif

#endif  // SKETCHUP_COMMON_H_
