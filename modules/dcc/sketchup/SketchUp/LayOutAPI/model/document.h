// Copyright 2015-2023 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_DOCUMENT_H_
#define LAYOUT_MODEL_DOCUMENT_H_

#include <LayOutAPI/common.h>
#include <LayOutAPI/model/defs.h>
#include <LayOutAPI/model/imagerep.h>
#include <LayOutAPI/model/sketchupmodel.h>
#include <time.h>

/**
@struct LODocumentRef
@brief References a LayOut document.
*/

/**
@enum LODocumentVersion
@brief Defines the different versions of the .layout file format that are
       supported.
*/
typedef enum {
  LODocumentVersion_1 = 1,      ///< LayOut 1.0
  LODocumentVersion_2 = 2,      ///< LayOut 2.0
  LODocumentVersion_3 = 3,      ///< LayOut 3.0
  LODocumentVersion_2013 = 13,  ///< LayOut 2013
  LODocumentVersion_2014 = 14,  ///< LayOut 2014
  LODocumentVersion_2015 = 15,  ///< LayOut 2015
  LODocumentVersion_2016 = 16,  ///< LayOut 2016
  LODocumentVersion_2017 = 17,  ///< LayOut 2017
  LODocumentVersion_2018 = 18,  ///< LayOut 2018
  LODocumentVersion_2019 = 19,  ///< LayOut 2019
  LODocumentVersion_2020 = 20,  ///< LayOut 2020
  LODocumentVersion_2021 = 21,  ///< LayOut 2021
  LODocumentVersion_2022 = 22,  ///< LayOut 2022
  LODocumentVersion_2023 = 23,  ///< LayOut 2023

  LODocumentVersion_Current = LODocumentVersion_2023,  ///< The most current version supported.

  LONumDocumentVersions
} LODocumentVersion;

/**
@enum LODocumentUnits
@brief Defines the different units formats that are available.
*/
typedef enum {
  LODocumentUnits_FractionalInches = 0,  ///< 6-1/2"
  LODocumentUnits_DecimalInches,         ///< 6.5"
  LODocumentUnits_DecimalFeet,           ///< 0.54167'
  LODocumentUnits_DecimalMillimeters,    ///< 165.1 mm
  LODocumentUnits_DecimalCentimeters,    ///< 16.51 cm
  LODocumentUnits_DecimalMeters,         ///< 1.651 m
  LODocumentUnits_DecimalPoints,         ///< 468 pt

  LONumDocumentUnits
} LODocumentUnits;

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Creates a new empty document object. It will contain one page and one
       layer.
@param[out] document The newly created document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *document already refers to a valid object
*/
LO_RESULT LODocumentCreateEmpty(LODocumentRef* document);

/**
@brief Creates a new document object by loading an existing .layout file.
@param[out] document The document created from the .layout file.
@param[in]  path     The path to the .layout file on disk.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *document already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_NO_DATA if the .layout file could not be found
- \ref SU_ERROR_SERIALIZATION if there was an error reading the .layout file
*/
LO_RESULT LODocumentCreateFromFile(LODocumentRef* document, const char* path);

/**
@brief Creates a new document object using an existing .layout file as a
       template. This differs from LODocumentCreateFromFile in that the new
       document won't have a path until it is saved for the first time.
@param[out] document The document created using the .layout file as a template.
@param[in]  path     The path to the layout template file on disk.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_OUTPUT if document is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *document already refers to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_SERIALIZATION if there was an error reading the template file
- \ref SU_ERROR_NO_DATA if the .layout file could not be found
*/
LO_RESULT LODocumentCreateFromTemplate(LODocumentRef* document, const char* path);

/**
 @brief Releases a document object. The object will be invalidated if
     releasing the last reference. Note: it is important to call
     LODocumentRelease before calling \ref LOTerminate, otherwise
     LODocumentRelease may fail unpredictably.
@param[in] document The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_NULL_POINTER_INPUT if document is NULL
- \ref SU_ERROR_INVALID_INPUT if *document does not refer to a valid object
*/
LO_RESULT LODocumentRelease(LODocumentRef* document);

/**
@brief Adds a reference to an document object.
@since LayOut 2018, API 3.0
@param[in] document The document object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
*/
LO_RESULT LODocumentAddReference(LODocumentRef document);

/**
@brief Saves a document to a file at the given path. Passing an empty path
       string will save the document at its current path.
@param[in] document The document object.
@param[in] path     The file path where the document should be saved.
@param[in] version  The file version to save the file as.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_OUT_OF_RANGE if version is not a valid version
- \ref SU_ERROR_SERIALIZATION if there was an error writing the file. This may
  be due to the LayOut file being open in the LayOut application
*/
LO_RESULT LODocumentSaveToFile(LODocumentRef document, const char* path, LODocumentVersion version);

/**
@brief Exports the document to a .pdf file at the indicated path. If a valid
       options dictionary object is provided, then it can specify additional
       PDF export options. The following options (declared in
       documentexportoptions.h) may be specified in the options dictionary to
       override their default values:
- <b>key: \ref LOExportOption_StartPage, value: Int32</b>\n
  Start page index. Defaults to 0.
- <b>key: \ref LOExportOption_EndPage, value: Int32</b>\n
  End page index. Defaults to the index of the last page in the document.
- <b>key: \ref LOExportOption_CompressImages, value: Bool</b>\n
  If true, images (including raster models) will be compressed using JPEG
  compression to reduce PDF file size. Defaults to true.
- <b>key: \ref LOExportOption_ImageCompressionQuality, value: Double</b>\n
  Specifies the image compression quality from 0.0 to 1.0, to be used if
  the \ref LOExportOption_CompressImages option is set to true. Defaults to
  0.5.

@param[in] document      The document object.
@param[in] path          The file path where the pdf should be exported.
@param[in] options_dict  The options dictionary. If the dictionary object is
                         invalid then default options will be used.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if path is NULL
- \ref SU_ERROR_GENERIC if an option was specified using a value of the wrong
  type
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_StartPage option
  specifies a start page that is out of range
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_EndPage option
  specifies an end page that is out of range
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_ImageCompressionQuality
  option specifies a value that is out of range
- \ref SU_ERROR_SERIALIZATION if there was an error writing the file
*/
LO_RESULT LODocumentExportToPDF(
    LODocumentRef document, const char* path, LODictionaryRef options_dict);


/**
@brief Exports the document as a series of image files at the indicated path.
       For multi-page documents, each page will append its page number to
       base_name to use as the file name. If a valid options dictionary object
       is provided, then it can specify additional image export options. The
       following options (declared in documentexportoptions.h) may be specified
       in the options dictionary to override their default values:
- <b>key: \ref LOExportOption_StartPage, value: Int32</b>\n
  Start page index. Defaults to 0.
- <b>key: \ref LOExportOption_EndPage, value: Int32</b>\n
  End page index. Defaults to the index of the last page in the document.
- <b>key: \ref LOExportOption_DPI, value: Int32</b>\n
  Image DPI. This will determine the pixel dimensions of the output images,
  in conjunction with the document's page width and height. Defaults to 96.

@param[in] document      The document object.
@param[in] export_path   The directory path to where the images should be
                         exported.
@param[in] base_name     The base name for the image files.
@param[in] format        What file format the images should be exported to.
@param[in] options_dict  The options dictionary. If the dictionary object is
                         invalid then default options will be used.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if export_path is NULL
- \ref SU_ERROR_NULL_POINTER_INPUT if base_name is NULL
- \ref SU_ERROR_OUT_OF_RANGE if format is out of range
- \ref SU_ERROR_GENERIC if an option was specified using a value of the wrong
  type
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_StartPage option
  specifies a start page that is out of range
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_EndPage option
  specifies an end page that is out of range
- \ref SU_ERROR_OUT_OF_RANGE if the \ref LOExportOption_DPI option specifies a
  DPI that is is less than 1 or greater than 1200
- \ref SU_ERROR_SERIALIZATION if there was an error writing the file(s)
*/
LO_RESULT LODocumentExportToImageSet(
    LODocumentRef document, const char* export_path, const char* base_name,
    LOImageRepOutputFormat format, LODictionaryRef options_dict);

/**
@brief Gets the full path to a document's .layout file.
@param[in]  document The document object.
@param[out] path     The path to the document's .layout file.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NO_DATA if document is a new document that does not yet have a
  path
- \ref SU_ERROR_NULL_POINTER_OUTPUT if path is NULL
- \ref SU_ERROR_INVALID_OUTPUT if *path does not refer to a valid string
  object.
*/
LO_RESULT LODocumentGetFilePath(LODocumentRef document, SUStringRef* path);

/**
@brief Gets the number of layers in a document.
@param[in]  document   The document object.
@param[out] num_layers The number of layers in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_layers is NULL
*/
LO_RESULT LODocumentGetNumberOfLayers(LODocumentRef document, size_t* num_layers);

/**
@brief Gets a reference to the layer definition at the given index for a
       document.
@param[in]  document         The document object.
@param[in]  index            The index of the layer to get.
@param[out] layer_definition The layer definition at the given index.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_definition is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_definition already refers to a valid
  object
*/
LO_RESULT LODocumentGetLayerAtIndex(
    LODocumentRef document, size_t index, LOLayerRef* layer_definition);

/**
@brief Populates a \ref LOLayerListRef with all of the layer definitions in a
       document.
@param[in]  document          The document object.
@param[out] layer_definitions The list to populate with the layer definitions
                              in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if layer_definitions does not refer to a valid
  object
*/
LO_RESULT LODocumentGetLayers(LODocumentRef document, LOLayerListRef layer_definitions);

/**
@brief Gets a reference to the document's active layer definition.
@since LayOut 2018, API 3.0
@param[in]  document         The document object.
@param[out] layer_definition The active layer definition in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_definition is NULL
*/
LO_RESULT LODocumentGetActiveLayer(LODocumentRef document, LOLayerRef* layer_definition);

/**
@brief Sets the document's active layer definition.
@since LayOut 2018, API 3.0
@param[in] document         The document object.
@param[in] layer_definition The layer definition to set as active.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_GENERIC if layer_definition does not belong to document
- \ref SU_ERROR_GENERIC if layer_definition is locked the active page
*/
LO_RESULT LODocumentSetActiveLayer(LODocumentRef document, LOLayerRef layer_definition);

/**
@brief Moves a layer to a different index within a document's list of layers.
       This will move the layer such that its new index becomes new_index.
@param[in] document         The document object.
@param[in] layer_definition The layer definition object.
@param[in] new_index        The index to move the layer to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_GENERIC if layer_definition does not belong to document
- \ref SU_ERROR_OUT_OF_RANGE if new_index is out of range
*/
LO_RESULT LODocumentReorderLayer(
    LODocumentRef document, LOLayerRef layer_definition, size_t new_index);
/**
@brief Adds a new layer to the document. The user is not responsible for
       releasing the returned layer definition object.
@param[in]  document         The document object.
@param[in]  shared           Whether or not the new layer should be a shared
                             layer.
@param[out] layer_definition The new layer definition object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if layer_definition is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *layer_definition already refers to a valid
  object
*/
LO_RESULT LODocumentAddLayer(LODocumentRef document, bool shared, LOLayerRef* layer_definition);

/**
@brief Removes a layer from a document. There must be at least one other
       unlocked and visible layer on every page. If this is not the case, then
       the next layer will be automatically unlocked and made visible on all
       pages as necessary to proceed with the operation. *layer_definition will
       be set to invalid by this function.
@param[in] document         The document object.
@param[in] layer_definition The layer definition object.
@param[in] delete_entities  Whether or not to delete the entities on the layer
                            that is being removed. If the entities are not
                            deleted, they will be moved to the next valid
                            layer. This may cause groups to be split if the
                            next valid layer does not have the same sharedness
                            as the layer being removed.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if layer_definition is NULL
- \ref SU_ERROR_INVALID_INPUT if *layer_definition does not refer to a valid
  object
- \ref SU_ERROR_GENERIC if *layer_definition does not belong to document
- \ref SU_ERROR_GENERIC if *layer_definition could not be removed because it is
  the last layer in the document
- \ref SU_ERROR_GENERIC if the layer could not be removed because it would
  break the rule that there must be at least one unlocked, visible layer on
  each page
*/
LO_RESULT LODocumentRemoveLayer(
    LODocumentRef document, LOLayerRef* layer_definition, bool delete_entities);

/**
@brief Gets the page info object for a document.
@param[in]  document  The document object.
@param[out] page_info The page info object for the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page_info is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page_info already refers to a valid object
*/
LO_RESULT LODocumentGetPageInfo(LODocumentRef document, LOPageInfoRef* page_info);

/**
@brief Gets the grid object for a document.
@param[in]  document The document object.
@param[out] grid     The grid object for the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if grid is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *grid already refers to a valid object
*/
LO_RESULT LODocumentGetGrid(LODocumentRef document, LOGridRef* grid);

/**
@brief Gets the total number of pages in a document.
@param[in]  document  The document object.
@param[out] num_pages The number of pages.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_pages is NULL
*/
LO_RESULT LODocumentGetNumberOfPages(LODocumentRef document, size_t* num_pages);

/**
@brief Gets the page at a given index in a document.
@param[in]  document The document object.
@param[in]  index    The index of the page to get.
@param[out] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is out of range
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page already refers to a valid object
*/
LO_RESULT LODocumentGetPageAtIndex(LODocumentRef document, size_t index, LOPageRef* page);

/**
@brief Populates a \ref LOPageListRef object with all the pages in a document.
@param[in]  document The document object.
@param[out] pages    The list to populate with the pages in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if pages does not refer to a valid object
*/
LO_RESULT LODocumentGetPages(LODocumentRef document, LOPageListRef pages);

/**
@brief Gets a reference to the page that will be shown the next time the
       document is opened in LayOut.
@since LayOut 2018, API 3.0
@param[in]  document The document object.
@param[out] page     The active page in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page is NULL
*/
LO_RESULT LODocumentGetInitialPage(LODocumentRef document, LOPageRef* page);

/**
@brief Sets the page that will be shown the next time the document is opened in
       LayOut.
@since LayOut 2018, API 3.0
@param[in] document The document object.
@param[in] page     The page to set as active.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_GENERIC if page does not belong to document
*/
LO_RESULT LODocumentSetInitialPage(LODocumentRef document, LOPageRef page);

/**
@brief Adds a new page to a document. The user is not responsible for releasing
       the returned page object.
@param[in]  document The document object.
@param[out] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if page is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *page already refers to a valid object
*/
LO_RESULT LODocumentAddPage(LODocumentRef document, LOPageRef* page);

/**
@brief Removes a page from a document. *page will be set to invalid by this
       function.
@param[in] document The document object.
@param[in] page     The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if page is NULL
- \ref SU_ERROR_INVALID_INPUT if *page does not refer to a valid object
- \ref SU_ERROR_GENERIC if *page does not belong to document
- \ref SU_ERROR_GENERIC if *page could not be removed because it is the last
  page in the document
*/
LO_RESULT LODocumentRemovePage(LODocumentRef document, LOPageRef* page);

/**
@brief Moves a page to a different index within a document's list of pages.
       This will move the page such that its new index becomes new_index.
@param[in] document  The document object.
@param[in] page      The page object.
@param[in] new_index The index the page should be moved to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object
- \ref SU_ERROR_GENERIC if page does not belong to document
- \ref SU_ERROR_OUT_OF_RANGE if new_index is out of range
*/
LO_RESULT LODocumentReorderPage(LODocumentRef document, LOPageRef page, size_t new_index);

/**
@brief Adds an entity to a document and place it on the given layer and page.
       If the given layer is a shared layer then page may be an invalid object.
       The entity must not already belong to a document. If entity is a group,
       then the group along with all of its children will be added to the
       document.
@param[in] document         The document object.
@param[in] entity           The entity object.
@param[in] layer_definition The layer definition object.
@param[in] page             The page object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if layer_definition does not refer to a valid
  object
- \ref SU_ERROR_INVALID_INPUT if page does not refer to a valid object and
  layer_definition refers to a non-shared layer
- \ref SU_ERROR_GENERIC if page or layer_definition does not belong to document
- \ref SU_ERROR_GENERIC if entity already belongs to a document
*/
LO_RESULT LODocumentAddEntity(
    LODocumentRef document, LOEntityRef entity, LOLayerRef layer_definition, LOPageRef page);

/**
@brief Adds an entity to a document and places it on the layer at layer_index
       and page at page_index. If the specified layer is a shared layer then
       page_index will be ignored. The entity must not already belong to a
       document. If entity is a group, then the group along with all of its
       children will be added to the document.
@param[in] document    The document object.
@param[in] entity      The entity object.
@param[in] layer_index The layer definition index.
@param[in] page_index  The page index.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT if entity does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if layer_index is not a valid value
- \ref SU_ERROR_OUT_OF_RANGE if page_index is not a valid value and the layer at
  layer_index is a non-shared layer
- \ref SU_ERROR_GENERIC if entity already belongs to a document
*/
LO_RESULT LODocumentAddEntityUsingIndexes(
    LODocumentRef document, LOEntityRef entity, size_t layer_index, size_t page_index);

/**
@brief Removes an entity from a document. If entity is a group, then the group
       and all of its children will be removed from the document.
@param[in] document The document object.
@param[in] entity   The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if entity is NULL
- \ref SU_ERROR_INVALID_INPUT if *entity does not refer to a valid object
- \ref SU_ERROR_GENERIC if entity does not belong to document
*/
LO_RESULT LODocumentRemoveEntity(LODocumentRef document, LOEntityRef* entity);

/**
@brief Gets the number of entities on shared layers at the top of a document's
       group hierarchy. This count will include \ref LOGroupRef entities so the
       group hierarchy can be traversed.
@param[in]  document            The document object.
@param[out] num_shared_entities The number of shared entities.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if num_shared_entities is NULL
*/
LO_RESULT LODocumentGetNumberOfSharedEntities(LODocumentRef document, size_t* num_shared_entities);

/**
@brief Gets the shared entity at the top of a document's group hierarchy at the
       specified index.
@param[in]  document The document object.
@param[in]  index    The 0-based entity index for the desired shared entity.
@param[out] entity   The entity object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if entity is NULL
- \ref SU_ERROR_OVERWRITE_VALID if *entity already refers to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if index is greater than or equal to the number of
  shared entities returned by LODocumentGetNumberOfSharedEntities
*/
LO_RESULT LODocumentGetSharedEntityAtIndex(
    LODocumentRef document, size_t index, LOEntityRef* entity);

/**
@brief Populates a \ref LOEntityListRef with the entities on shared layers
       at the top of a document's group hierarchy. This will include
       \ref LOGroupRef entities so the group hierarchy can be traversed.
@param[in] document    The document object.
@param[in] entity_list The entity list to populate.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if entity_list does not refer to a valid object
*/
LO_RESULT LODocumentGetSharedEntities(LODocumentRef document, LOEntityListRef entity_list);

/**
@brief Gets whether or not object snap is enabled in a document.
@param[in]  document    The document object.
@param[out] object_snap Whether object snap is enabled or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if object_snap is NULL
*/
LO_RESULT LODocumentGetObjectSnap(LODocumentRef document, bool* object_snap);

/**
@brief Sets whether or not object snap is enabled in a document.
@param[in] document    The document object.
@param[in] object_snap Whether to enable or disable object snap.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
*/
LO_RESULT LODocumentSetObjectSnap(LODocumentRef document, bool object_snap);

/**
@brief Gets whether or not grid snap is enabled in a document.
@param[in]  document  The document object.
@param[out] grid_snap Whether grid snap is enabled or not.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if grid_snap is NULL
*/
LO_RESULT LODocumentGetGridSnap(LODocumentRef document, bool* grid_snap);

/**
@brief Sets whether or not grid snap is enabled in a document.
@param[in] document  The document object.
@param[in] grid_snap Whether to enable or disable grid snap.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
*/
LO_RESULT LODocumentSetGridSnap(LODocumentRef document, bool grid_snap);

/**
@brief Returns the time at which a document was created.
@param[in]  document     The document object.
@param[out] time_created The time when the document was created.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if time_created is NULL
*/
LO_RESULT LODocumentGetTimeCreated(LODocumentRef document, time_t* time_created);

/**
@brief Returns the last time that the document was edited.
@param[in]  document      The document object.
@param[out] time_modified The last time the document was edited.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if time_modified is NULL
*/
LO_RESULT LODocumentGetTimeModified(LODocumentRef document, time_t* time_modified);

/**
@brief Returns the last time that the document was printed or exported.
@param[in]  document       The document object.
@param[out] time_published The last time the document was printed or exported.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if time_published is NULL
*/
LO_RESULT LODocumentGetTimePublished(LODocumentRef document, time_t* time_published);

/**
@brief Gets the units and precision for a document.
@param[in]  document  The document object.
@param[out] units     The units setting for the document.
@param[out] precision The units precision. This is expressed as a value in the
                      current units.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if units is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if precision is NULL
*/
LO_RESULT LODocumentGetUnits(LODocumentRef document, LODocumentUnits* units, double* precision);

/**
@brief Sets the units and precision for a document.
@param[in] document  The document object.
@param[in] units     The units setting for the document.
@param[in] precision The units precision. This is expressed as a value in the
                     specified units. LayOut only allows for a finite set of
                     precision values for each units setting, so it will set
                     the precision to the closest valid setting for the
                     specified units. See the "Units" section of LayOut's
                     "Document Setup" dialog for a reference of the available
                     precisions for each units setting.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_OUT_OF_RANGE if units is invalid
*/
LO_RESULT LODocumentSetUnits(LODocumentRef document, LODocumentUnits units, double precision);


/**
@brief Gets the number of auto-text definitions in a document.
@since LayOut 2017, API 2.0
@param[in]  document The document object.
@param[out] size     The amount of auto-text definitions in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if size is NULL
*/
LO_RESULT LODocumentGetNumberOfAutoTextDefinitions(LODocumentRef document, size_t* size);

/**
@brief Gets the auto-text at the specified index.
@since LayOut 2017, API 2.0
@param[in]  document The document object.
@param[in]  index    The index of the auto-text object.
@param[out] autotext The auto-text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if autotext is NULL
- \ref SU_ERROR_OUT_OF_RANGE if index is not a valid index
*/
LO_RESULT LODocumentGetAutoTextDefinitionAtIndex(
    LODocumentRef document, size_t index, LOAutoTextDefinitionRef* autotext);

/**
@brief Gets the auto-text with the specified name.
@since LayOut 2018, API 3.0
@param[in]  document The document object.
@param[in]  name     The name of the auto-text object to get.
@param[out] autotext The auto-text object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if name is NULL
- \ref SU_ERROR_NULL_POINTER_OUTPUT if autotext is NULL
- \ref SU_ERROR_NO_DATA if no auto-text definition exists with the given name
*/
LO_RESULT LODocumentGetAutoTextDefinitionByName(
    LODocumentRef document, const char* name, LOAutoTextDefinitionRef* autotext);

/**
@brief Populates a \ref LOAutoTextDefinitionListRef object with all the auto-text
       in a document.
@since LayOut 2018, API 3.0
@param[in]  document  The document object.
@param[out] autotexts The list to populate with the auto-texts in the document.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_OUTPUT if auto_texts does not refer to a valid object
*/
LO_RESULT LODocumentGetAutoTextDefinitions(
    LODocumentRef document, LOAutoTextDefinitionListRef autotexts);

/**
@brief Adds an auto-text definition to a document.
@since LayOut 2017, API 2.0
@param[in]  document The document object.
@param[in]  type     The desired type of the new auto-text.
@param[in]  name     The desired name of the new auto-text.
@param[out] autotext The autotext definition object.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_INVALID_INPUT is autotext does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if index is NULL
- \ref SU_ERROR_OUT_OF_RANGE if type is not a valid type
- \ref SU_ERROR_UNSUPPORTED if type is a mandatory auto-text type
*/
LO_RESULT LODocumentAddAutoTextDefinition(
    LODocumentRef document, int type, const char* name, LOAutoTextDefinitionRef* autotext);

/**
@brief Removes an auto-text definition from a document.
@since LayOut 2017, API 2.0
@param[in] document                    The document object.
@param[in] autotext                    The auto-text definition object.
@param[in] convert_tags_to_normal_text True if the auto-text being deleted
                                       should retain its tags in normal text.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document does not refer to a valid object
- \ref SU_ERROR_NULL_POINTER_INPUT if autotext is NULL
- \ref SU_ERROR_INVALID_INPUT if autotext does not refer to a valid object
- \ref SU_ERROR_UNSUPPORTED if autotext is a mandatory auto-text definition
- \ref SU_ERROR_INVALID_ARGUMENT if autotext does not belong to this document
*/
LO_RESULT LODocumentRemoveAutoTextDefinition(
    LODocumentRef document, LOAutoTextDefinitionRef* autotext, bool convert_tags_to_normal_text);

/**
@brief Gets the length formatter settings from the document. The given length
       formatter object must have been constructed using \ref
       SULengthFormatterCreate. It must be released using \ref
       SULengthFormatterRelease.
@since LayOut 2018, API 3.0
@param[in]  document  The document object.
@param[out] formatter The formatter to apply the document's settings to.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if document is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if formatter is NULL
- \ref SU_ERROR_INVALID_OUTPUT if formatter does not point to a valid \ref
       SULengthFormatterRef object
*/
LO_RESULT LODocumentGetLengthFormatter(LODocumentRef document, SULengthFormatterRef* formatter);


/**
@brief Gets the render mode override for export of SketchUp models in the document.

This is used to override the render mode for all \ref LOSketchUpModelRef s in the document when
exporting. To have the output render mode match each \ref LOSketchUpModelRef 's edit render mode,
set this to LOSketchUpModelRenderMode_NoOverride.
@since LayOut 2023.1, API 8.1
@param[in] document     The document object.
@param[out] render_mode The render mode of the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p document is not a valid object
- \ref SU_ERROR_NULL_POINTER_OUTPUT if \p render_mode is NULL
*/
LO_RESULT LODocumentGetRenderModeOverride(
    LODocumentRef document, LOSketchUpModelRenderMode* render_mode);

/**
@brief Sets the render mode override for export of SketchUp models in the document.

This is used to override the render mode for all raster rendered \ref LOSketchUpModelRef s in the
document when exporting. To have the output render mode match each \ref LOSketchUpModelRef 's edit
render mode, set this to LOSketchUpModelRenderMode_NoOverride.
@since LayOut 2023.1, API 8.1
@param[in] document     The document object.
@param[in] render_mode The new render mode for the SketchUp model.
@return
- \ref SU_ERROR_NONE on success
- \ref SU_ERROR_INVALID_INPUT if \p document is not a valid object
- \ref SU_ERROR_OUT_OF_RANGE if the value of \p render_mode is invalid
- \ref SU_ERROR_INVALID_ARGUMENT if the value of \p render_mode is \ref
    LOSketchUpModelRenderMode_Raster
*/
LO_RESULT LODocumentSetRenderModeOverride(
    LODocumentRef document, LOSketchUpModelRenderMode render_mode);
#ifdef __cplusplus
}  // extern "C" {
#endif

#endif  // LAYOUT_MODEL_DOCUMENT_H_
