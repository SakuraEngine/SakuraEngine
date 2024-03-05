// Copyright 2015 Trimble Navigation Ltd. All Rights Reserved

#ifndef LAYOUT_INITIALIZE_H_
#define LAYOUT_INITIALIZE_H_

#include <LayOutAPI/common.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Initializes the LayOut C API interface. Must be called before calling any
other API function.
*/
LO_EXPORT void LOInitialize();

/**
@brief Signals termination of use of the LayOut C API interface. Must be called
when done using API functions.
*/
LO_EXPORT void LOTerminate();

/**
@brief Returns the major and minor API version numbers.
@param[out] major The major version number retrieved.
@param[out] minor The minor version number retrieved.
*/
LO_EXPORT void LOGetAPIVersion(size_t* major, size_t* minor);


#ifdef __cplusplus
}  //  extern "C" {
#endif

#endif  // LAYOUT_INITIALIZE_H_
