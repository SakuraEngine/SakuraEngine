// Copyright 2021 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Type for SketchUp model file version.
 */
#ifndef SKETCHUP_MODEL_MODEL_VERSION_H_
#define SKETCHUP_MODEL_MODEL_VERSION_H_

#pragma pack(push, 8)

#ifdef __cplusplus
extern "C" {
#endif

/**
@enum SUModelVersion
@brief SketchUp model file format version
@since SketchUp 2014, API 2.0
@note Starting with SketchUp 2021, SketchUp is using a the same file format across versions.
  For instance, SketchUp 2021 can open a file made in SketchUp 2022.
*/
enum SUModelVersion {
  SUModelVersion_SU3,     ///< SketchUp 3
  SUModelVersion_SU4,     ///< SketchUp 4
  SUModelVersion_SU5,     ///< SketchUp 5
  SUModelVersion_SU6,     ///< SketchUp 6
  SUModelVersion_SU7,     ///< SketchUp 7
  SUModelVersion_SU8,     ///< SketchUp 8
  SUModelVersion_SU2013,  ///< SketchUp 2013
  SUModelVersion_SU2014,  ///< SketchUp 2014
  SUModelVersion_SU2015,  ///< SketchUp 2015
  SUModelVersion_SU2016,  ///< SketchUp 2016
  SUModelVersion_SU2017,  ///< SketchUp 2017
  SUModelVersion_SU2018,  ///< SketchUp 2018
  SUModelVersion_SU2019,  ///< SketchUp 2019
  SUModelVersion_SU2020,  ///< SketchUp 2020
  SUModelVersion_SU2021,  ///< "Versionless" file format. Starting with SketchUp 2021.
  SUModelVersion_Current = SUModelVersion_SU2021  ///< The most current version supported.
                                                  ///< Added in SketchUp 2022, API 10.0.
};

#ifdef __cplusplus
}  // extern "C"
#endif

#pragma pack(pop)

#endif  // SKETCHUP_MODEL_MODEL_VERSION_H_
