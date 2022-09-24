#pragma once
#include "edcore_configure.h"
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QtGui/qopenglext.h>

namespace skq
{
struct SKR_EDITOR_CORE_API SQGLFunctions
{
    static void Initialize(QOpenGLContext*) noexcept;

    static PFNGLCREATEMEMORYOBJECTSEXTPROC pGLCreateMemoryObjectsEXT;
    static PFNGLDELETEMEMORYOBJECTSEXTPROC pGLDeleteMemoryObjectsEXT;
#ifdef _WIN32
    static PFNGLIMPORTMEMORYWIN32NAMEEXTPROC pGLImportMemoryWin32NameEXT;
#else
    static PFNGLIMPORTMEMORYFDEXTPROC pGLImportMemoryFdEXT;
#endif
    static PFNGLCREATETEXTURESPROC pGLCreateTextures;
    static PFNGLDELETETEXTURESEXTPROC pGLDeleteTextures;
    
    static PFNGLTEXTURESTORAGEMEM2DEXTPROC pGLTextureStorageMem2DEXT;
};
}