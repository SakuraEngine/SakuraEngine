// Copyright 2016-2022 Trimble Inc. All Rights Reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_AUTOTEXTDEFINITION_H_
#define LAYOUT_MODEL_AUTOTEXTDEFINITION_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>

/**
 @struct LOAutoTextDefinitionRef
 @brief References an auto-text definition.
 @since LayOut 2017, API 2.0
 */

/**
 @enum LOAutoTextDefinitionType
 @brief Defines the different types of auto-text.
 @since LayOut 2017, API 2.0
 */
typedef enum {
  LOAutoTextDefinitionType_File = 0,
  LOAutoTextDefinitionType_PageName,
  LOAutoTextDefinitionType_PageNumber,
  LOAutoTextDefinitionType_CustomText,
  LOAutoTextDefinitionType_DateCurrent,
  LOAutoTextDefinitionType_DateCreated,
  LOAutoTextDefinitionType_DateModified,
  LOAutoTextDefinitionType_DatePublished,

  // Mandatory types
  LOAutoTextDefinitionType_ModelGroupName,
  LOAutoTextDefinitionType_ModelComponentInstanceName,
  LOAutoTextDefinitionType_ModelComponentDefinitionName,
  LOAutoTextDefinitionType_ModelComponentDescription,
  LOAutoTextDefinitionType_ModelVolume,
  LOAutoTextDefinitionType_ModelFaceArea,
  LOAutoTextDefinitionType_ModelEdgeLength,
  LOAutoTextDefinitionType_ModelCoordinates,
  LOAutoTextDefinitionType_ModelDynamicComponentAttribute,
  LOAutoTextDefinitionType_ModelClassifierAttribute,
  LOAutoTextDefinitionType_ModelComponentInstanceAttribute,
  LOAutoTextDefinitionType_ModelComponentDefinitionAttribute,

  // @since LayOut 2022.0, API 7.0
  LOAutoTextDefinitionType_ModelSceneName,
  LOAutoTextDefinitionType_ModelSceneDescription,
  LOAutoTextDefinitionType_ModelScale,
  LOAutoTextDefinitionType_ModelSectionName,
  LOAutoTextDefinitionType_ModelSectionSymbol,
  LOAutoTextDefinitionType_PageCount,
  LOAutoTextDefinitionType_Sequence,
  LOAutoTextDefinitionType_ModelRatio,

  // Must be last to give us a count of the number of types.
  LOAutoTextDefinitionType_Count
} LOAutoTextDefinitionType;

/**
 @enum LOAutoTextDefinitionNumberingStyle
 @brief An enumeration of the different styles of page numbers we support.
 @since LayOut 2017, API 2.0
 */
typedef enum {
  LOAutoTextDefinitionNumberingStyle_Arabic = 0,
  LOAutoTextDefinitionNumberingStyle_ArabicPadded,
  LOAutoTextDefinitionNumberingStyle_UCAlpha,
  LOAutoTextDefinitionNumberingStyle_LCAlpha,
  LOAutoTextDefinitionNumberingStyle_UCRoman,
  LOAutoTextDefinitionNumberingStyle_LCRoman,

  // Must be last to give us a count of styles.
  LOAutoTextDefinitionNumberingStyle_Count
} LOAutoTextDefinitionNumberingStyle;

/**
 @enum LOAutoTextDefinitionSequenceType
 @brief An enumeration of the different types of auto-text sequencing.
 @since LayOut 2023.0, API 8.0
 */
typedef enum {
  LOAutoTextDefinitionSequenceType_PerDocument = 0,
  LOAutoTextDefinitionSequenceType_PerPage,

  // Must be last to give us a count of sequence types.
  LOAutoTextDefinitionSequenceType_Count
} LOAutoTextDefinitionSequenceType;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
@brief Gets the type of the specified auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext The auto-text definition object.
@param[out] type     The auto-text definition type.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if type is NULL
*/
LO_RESULT LOAutoTextDefinitionGetType(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionType* type);

/**
@brief Gets whether the specified auto-text definition is a mandatory definition.
       Mandatory auto-text definitions may not be removed from a document, and
       they may not be created through the API, but they may be modified.
@since LayOut 2018, API 3.0
@param[in]  autotext     The auto-text definition object.
@param[out] is_mandatory Whether it is a mandatory auto-text definition.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_mandatory is NULL
*/
LO_RESULT LOAutoTextDefinitionGetMandatory(LOAutoTextDefinitionRef autotext, bool* is_mandatory);

/**
@brief Returns the document that this auto-text definition belongs to.
@since LayOut 2018, API 3.0
@param[in]  autotext The auto-text definition object.
@param[out] document The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
*/
LO_RESULT LOAutoTextDefinitionGetDocument(
    LOAutoTextDefinitionRef autotext, LODocumentRef* document);

/**
@brief Gets the name of the specified auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext The auto-text definition object.
@param[out] name     The auto-text definition name string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if name is NULL
*/
LO_RESULT LOAutoTextDefinitionGetName(LOAutoTextDefinitionRef autotext, SUStringRef* name);

/**
@brief Sets the name of the specified auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext The auto-text definition object.
@param[out] name     The auto-text definition name string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext is a mandatory definition
- \ref SU_ERROR_DUPLICATE if name is the name of an existing definition
- \ref SU_ERROR_UNSUPPORTED if name is an empty string
*/
LO_RESULT LOAutoTextDefinitionSetName(LOAutoTextDefinitionRef autotext, const char* name);

/**
@brief Gets the tag string of the specified auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext The auto-text definition object.
@param[out] tag      The auto-text definition tag string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if tag is NULL
*/
LO_RESULT LOAutoTextDefinitionGetTag(LOAutoTextDefinitionRef autotext, SUStringRef* tag);

/**
@brief Gets the index of the specified auto-text definition.
@since LayOut 2018, API 3.0
@param[in]  autotext The auto-text definition object.
@param[out] index    The index of the auto-text definition.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if index is NULL
*/
LO_RESULT LOAutoTextDefinitionGetIndex(LOAutoTextDefinitionRef autotext, size_t* index);

/**
@brief Gets the text string of the specified custom auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext    The auto-text definition object.
@param[out] custom_text The auto-text definition custom text string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if custom_text is NULL
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not that of custom text
*/
LO_RESULT LOAutoTextDefinitionGetCustomText(
    LOAutoTextDefinitionRef autotext, SUStringRef* custom_text);

/**
@brief Sets the text string of the specified custom auto-text definition.
@since LayOut 2017, API 2.0
@param[in] autotext    The auto-text definition object.
@param[in] custom_text The auto-text definition custom text string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not that of custom text
*/
LO_RESULT LOAutoTextDefinitionSetCustomText(
    LOAutoTextDefinitionRef autotext, const char* custom_text);

/**
@brief Gets the date format string of the specified date auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext    The auto-text definition object.
@param[out] date_format The auto-text definition date format string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if date_format is NULL
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a date type
*/
LO_RESULT LOAutoTextDefinitionGetDateFormat(
    LOAutoTextDefinitionRef autotext, SUStringRef* date_format);

/**
@brief Sets the date format string of the specified date auto-text definition.
@since LayOut 2017, API 2.0
@param[in] autotext    The auto-text definition object.
@param[in] date_format The auto-text definition date format string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a date type
*/
LO_RESULT LOAutoTextDefinitionSetDateFormat(
    LOAutoTextDefinitionRef autotext, const char* date_format);

/**
@brief Gets a boolean signifying if the full path is displayed for the specified
       file auto-text definition.
@since LayOut 2017, API 2.0
@param[in] autotext             The auto-text definition object.
@param[in] is_display_full_path Boolean signifying if the full path is being
           displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_display_full_path is NULL
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a file type
*/
LO_RESULT LOAutoTextDefinitionGetDisplayFullPath(
    LOAutoTextDefinitionRef autotext, bool* is_display_full_path);

/**
@brief Sets whether the full path is displayed for the specified file auto-text
       definition.
@since LayOut 2017, API 2.0
@param[in] autotext             The auto-text definition object.
@param[in] is_display_full_path Boolean signifying if the full path is being
           displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a file type
*/
LO_RESULT LOAutoTextDefinitionSetDisplayFullPath(
    LOAutoTextDefinitionRef autotext, bool is_display_full_path);

/**
@brief Gets a boolean signifying if the file extension is displayed for the
       specified file auto-text definition.
@since LayOut 2017, API 2.0
@param[in] autotext             The auto-text definition object.
@param[in] is_display_file_ext  Boolean signifying if the full path is being
displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if is_display_file_ext is NULL
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a file type
*/
LO_RESULT LOAutoTextDefinitionGetDisplayFileExtension(
    LOAutoTextDefinitionRef autotext, bool* is_display_file_ext);

/**
@brief Sets whether the file extension is displayed for the specified file auto
       text definition.
@since LayOut 2017, API 2.0
@param[in] autotext             The auto-text definition object.
@param[in] is_display_file_ext  Boolean signifying if the full path is being
           displayed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a file type
*/
LO_RESULT LOAutoTextDefinitionSetDisplayFileExtension(
    LOAutoTextDefinitionRef autotext, bool is_display_file_ext);
/**
@brief Gets the numbering style of the specified page number auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext        The auto-text definition object.
@param[out] numbering_style The style of numbering.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p numbering_style is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number
*/
SU_DEPRECATED_FUNCTION("LayOut API 7.0")
LO_RESULT LOAutoTextDefinitionGetPageNumberStyle(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionNumberingStyle* numbering_style);

/**
@brief Sets the numbering style of the specified page number auto-text definition.
@since LayOut 2017, API 2.0
@param[in]  autotext        The auto-text definition object.
@param[out] numbering_style The style of numbering.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p numbering_style is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number
*/
SU_DEPRECATED_FUNCTION("LayOut API 7.0")
LO_RESULT LOAutoTextDefinitionSetPageNumberStyle(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionNumberingStyle numbering_style);

/**
@brief Gets the index at which the auto-text definition starts numbering.
@since LayOut 2017, API 2.0
@param[in]  autotext    The auto-text definition object.
@param[out] start_index The starting index.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p start_index is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number
*/
SU_DEPRECATED_FUNCTION("LayOut API 7.0")
LO_RESULT LOAutoTextDefinitionGetPageNumberStartIndex(
    LOAutoTextDefinitionRef autotext, int* start_index);

/**
@brief Sets the value at which to start counting.
@since LayOut 2017, API 2.0
@param[in]  autotext The auto-text definition object.
@param[out] value    The number at which to start counting.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if \p value is NULL
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a page number
*/
SU_DEPRECATED_FUNCTION("LayOut API 7.0")
LO_RESULT LOAutoTextDefinitionSetPageNumberStartIndex(LOAutoTextDefinitionRef autotext, int value);

/**
@brief Gets the page that the page number or page count auto-text definition starts numbering on.
@since LayOut 2022.0, API 7.0
@param[in]  autotext The auto-text definition object.
@param[out] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p page is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number, page count, or sequence
*/
LO_RESULT LOAutoTextDefinitionGetStartPage(LOAutoTextDefinitionRef autotext, LOPageRef* page);

/**
@brief Sets the page that the page number or page count auto-text definition starts numbering on.
@since LayOut 2022.0, API 7.0
@param[in] autotext The auto-text definition object.
@param[in] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if \p page does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number or page count
*/
LO_RESULT LOAutoTextDefinitionSetStartPage(LOAutoTextDefinitionRef autotext, LOPageRef page);

/**
@brief Gets the page the page count auto-text definition stops counting at. page may be returned as
       SU_INVALID, which indicates that the auto-text uses the last page in the document.
@since LayOut 2022.0, API 7.0
@param[in]  autotext The auto-text definition object.
@param[out] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p page is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page count
*/
LO_RESULT LOAutoTextDefinitionGetEndPage(LOAutoTextDefinitionRef autotext, LOPageRef* page);

/**
@brief Sets the page the page count auto-text definition stops counting at. This page must be after
       or the same as start page. If page is SU_INVALID, then the last page in the document will
       be used.
@since LayOut 2022.0, API 7.0
@param[in] autotext The auto-text definition object.
@param[in] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if \p page is a page that comes after this definition's start page
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page count
*/
LO_RESULT LOAutoTextDefinitionSetEndPage(LOAutoTextDefinitionRef autotext, LOPageRef page);

/**
@brief Gets the increment value of the specified sequence auto-text definition.
@since LayOut 2022.0, API 7.0
@param[in]  autotext  The auto-text definition object.
@param[out] increment The amount each sequence object increments by.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p increment is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not a sequence type
*/
LO_RESULT LOAutoTextDefinitionGetIncrement(LOAutoTextDefinitionRef autotext, int* increment);

/**
@brief Sets the increment value of the specified sequence auto-text definition.
@since LayOut 2022.0, API 7.0
@param[in] autotext  The auto-text definition object.
@param[in] increment The new amount each sequence object will increment by.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not a sequence type
*/
LO_RESULT LOAutoTextDefinitionSetIncrement(LOAutoTextDefinitionRef autotext, int increment);

/**
@brief Gets the sequence format string of the specified date auto-text definition.
@since LayOut 2022.0, API 7.0
@param[in]  autotext        The auto-text definition object.
@param[out] sequence_format The auto-text definition sequence format string.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p sequence_format is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not a sequence type
*/
LO_RESULT LOAutoTextDefinitionGetSequenceFormat(
    LOAutoTextDefinitionRef autotext, SUStringRef* sequence_format);

/**
@brief Sets the format string of the specified sequence auto-text definition. If the string is
       empty, the format string will be set to the default of ###.
@since LayOut 2022.0, API 7.0
@param[in] autotext        The auto-text definition object.
@param[in] sequence_format The new format string for the sequence.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p sequence_format is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not a sequence type
*/
LO_RESULT LOAutoTextDefinitionSetSequenceFormat(
    LOAutoTextDefinitionRef autotext, const char* sequence_format);

/**
@brief Iterates through all uses of the auto-text definition and eliminates gaps and duplicates in
the sequence.
@since LayOut 2022.0, API 7.0
@param[in] autotext The auto-text definition object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not a sequence type
*/
LO_RESULT LOAutoTextDefinitionRenumberSequence(LOAutoTextDefinitionRef autotext);

/**
@brief Gets the numbering style of the specified page number, page count, or sequence auto-text
       definition.
@since LayOut 2022.0, API 7.0
@note Use this method instead of the deprecated \p LOAutoTextDefinitionGetPageNumberStyle.
@param[in]  autotext        The auto-text definition object.
@param[out] numbering_style The style of numbering.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p numbering_style is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number, page count, or sequence
*/
LO_RESULT LOAutoTextDefinitionGetNumberStyle(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionNumberingStyle* numbering_style);

/**
@brief Sets the numbering style of the specified page number, page count, or sequence auto-text
       definition.
@since LayOut 2022.0, API 7.0
@note Use this method instead of the deprecated \p LOAutoTextDefinitionSetPageNumberStyle.
@param[in] autotext        The auto-text definition object.
@param[in] numbering_style The style of numbering.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number, page count, or sequence
- \ref SU_ERROR_OUT_OF_RANGE if \p numbering_style is not a valid numbering style
*/
LO_RESULT LOAutoTextDefinitionSetNumberStyle(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionNumberingStyle numbering_style);

/**
@brief Gets the index at which the auto-text definition starts numbering.
@since LayOut 2022.0, API 7.0
@note Use this method instead of the deprecated \p LOAutoTextDefinitionGetPageNumberStartIndex.
@param[in]  autotext    The auto-text definition object.
@param[out] start_index The starting index.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p start_index is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not page number, page count, or sequence
*/
LO_RESULT LOAutoTextDefinitionGetStartIndex(LOAutoTextDefinitionRef autotext, int* start_index);

/**
@brief Sets the value at which to start counting.
@since LayOut 2022.0, API 7.0
@note Use this method instead of the deprecated \p LOAutoTextDefinitionGetPageNumberStartIndex.
@param[in] autotext The auto-text definition object.
@param[in] value    The number at which to start counting.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext's type is not a page number, page count, or sequence type
*/
LO_RESULT LOAutoTextDefinitionSetStartIndex(LOAutoTextDefinitionRef autotext, int value);

/**
@brief Gets the sequence type of the specified sequence auto-text definition.
@since LayOut 2023.0, API 8.0
@param[in]  autotext      The auto-text definition object.
@param[out] sequence_type The sequencing behavior of this definition.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p sequence_type is NULL
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not sequence
*/
LO_RESULT LOAutoTextDefinitionGetSequenceType(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionSequenceType* sequence_type);

/**
@brief Sets the sequence type of the specified sequence auto-text definition.
@since LayOut 2023.0, API 8.0
@param[in] autotext      The auto-text definition object.
@param[in] sequence_type The sequencing behavior.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if \p autotext's type is not sequence
- \ref SU_ERROR_OUT_OF_RANGE if \p sequence_type is not a valid sequence type
*/
LO_RESULT LOAutoTextDefinitionSetSequenceType(
    LOAutoTextDefinitionRef autotext, LOAutoTextDefinitionSequenceType sequence_type);

#ifdef __cplusplus
}  // end extern "C"
#endif  // __cplusplus

#endif  // LAYOUT_MODEL_AUTOTEXTDEFINITION_H_
