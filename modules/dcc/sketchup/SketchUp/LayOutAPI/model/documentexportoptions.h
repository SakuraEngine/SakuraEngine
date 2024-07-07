// Copyright 2015-2023 Trimble Inc. All rights reserved.
// This file is intended for public distribution.

#ifndef LAYOUT_MODEL_DOCUMENTEXPORTOPTIONS_H_
#define LAYOUT_MODEL_DOCUMENTEXPORTOPTIONS_H_

/**
@brief Dictionary key to use when specifying the file location for export.
*/
const char* const LOExportOption_FileLocation = "lo_export_file_location";

/**
@brief Dictionary key to use when specifying the start page option for
       \ref LODocumentExportToPDF or \ref LODocumentExportToImageSet.
*/
const char* const LOExportOption_StartPage = "start_page";

/**
@brief Dictionary key to use when specifying the end page option for
       \ref LODocumentExportToPDF or \ref LODocumentExportToImageSet.
*/
const char* const LOExportOption_EndPage = "end_page";

/**
@brief Dictionary key to use when specifying the output resolution option for
       \ref LODocumentExportToPDF.
@deprecated This key is deprecated as of 2023.1. Set the output resolution of
            images and viewports through the LOPageInfoSetImageOutputResolution
            and LOPageInfoSetOutputResolution respectively.
*/
const char* const LOExportOption_OutputResolution = "output_resolution";

/**
@brief Dictionary key to use when specifying the compress images option for
       \ref LODocumentExportToPDF.
*/
const char* const LOExportOption_CompressImages = "compress_images";

/**
@brief Dictionary key to use when specifying the image compression quality
       option for \ref LODocumentExportToPDF.
*/
const char* const LOExportOption_ImageCompressionQuality = "compress_quality";

/**
@brief Dictionary key to use when specifying the image DPI option for
       \ref LODocumentExportToImageSet.
*/
const char* const LOExportOption_DPI = "dpi";

#endif  // #define LAYOUT_MODEL_DOCUMENTEXPORTOPTIONS_H_
