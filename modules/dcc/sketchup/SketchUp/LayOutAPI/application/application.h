// Copyright 2014-2022 Trimble Inc. All Rights Reserved.

#ifndef LAYOUT_APPLICATION_APPLICATION_H_
#define LAYOUT_APPLICATION_APPLICATION_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>


/**
@struct LOApplicationRef
@brief Encapsulates a LayOut application.
@since LayOut 2018, API 3.0
*/

DEFINE_SU_TYPE(LOApplicationRef)

#pragma pack(push, 8)
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus


/**
@brief Gets the application instance.
@since LayOut 2018, API 3.0
@param[out] app The application instance.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NO_DATA if we are not running inside an instance of the LayOut
  application
*/
LO_RESULT LOApplicationGetApplication(LOApplicationRef* app);

/**
@brief Gets a reference to the active document.
@since LayOut 2018, API 3.0
@param[in]  app      The application instance.
@param[out] document The active document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
*/
LO_RESULT LOApplicationGetActiveDocument(LOApplicationRef app, LODocumentRef* document);

/**
@brief Gets the major/minor version number as a string.
@since LayOut 2018, API 3.0
@param[in]  app     The application instance.
@param[out] version The current version of the application.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if version is NULL
*/
LO_RESULT LOApplicationGetVersion(LOApplicationRef app, SUStringRef* version);

/**
@brief Gets the major/minor version number as an unsigned integer.
@since LayOut 2018, API 3.0
@param[in]  app     The application instance.
@param[out] version The current version of the application.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if version is NULL
*/
LO_RESULT LOApplicationGetVersionNumber(LOApplicationRef app, unsigned int* version);

/**
@brief Gets whether the application is connected to the internet.
@since LayOut 2018, API 3.0
@param[in]  app       The application instance.
@param[out] is_online True if the application is connected to the internet.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_online is NULL
*/
LO_RESULT LOApplicationGetOnline(LOApplicationRef app, bool* is_online);

/**
@brief Opens the LayOut document located at the given file path.
@since LayOut 2018, API 3.0
@param[in]  app       The application instance.
@param[in]  file_path The full file path to the .layout document to open.
@param[out] document  A reference to the opened document if successful.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if file_path is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
- \ref SU_ERROR_SERIALIZATION if the document fails to open
*/
LO_RESULT LOApplicationOpenDocument(
    LOApplicationRef app, const char* file_path, LODocumentRef* document);


/**
@brief Quits the application.
@since LayOut 2018, API 3.0
@param[in] app The application instance.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if app is not a valid object
*/
LO_RESULT LOApplicationQuit(LOApplicationRef app);



#ifdef __cplusplus
}  // extern "C" {
#endif
#pragma pack(pop)

#endif  // LAYOUT_APPLICATION_APPLICATION_H_
