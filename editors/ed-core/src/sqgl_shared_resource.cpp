#include "editor_core/sqgl_shared_resource.h"
#include <QtGui/qopenglext.h>

PFNGLCREATEMEMORYOBJECTSEXTPROC skq::SQGLFunctions::pGLCreateMemoryObjectsEXT = nullptr;
PFNGLDELETEMEMORYOBJECTSEXTPROC skq::SQGLFunctions::pGLDeleteMemoryObjectsEXT = nullptr;

#ifdef _WIN32
PFNGLIMPORTMEMORYWIN32NAMEEXTPROC skq::SQGLFunctions::pGLImportMemoryWin32NameEXT = nullptr;
#else
PFNGLIMPORTMEMORYFDEXTPROC skq::SQGLFunctions::pGLImportMemoryFdEXT = nullptr;
#endif

PFNGLCREATETEXTURESPROC skq::SQGLFunctions::pGLCreateTextures = nullptr;
PFNGLDELETETEXTURESEXTPROC skq::SQGLFunctions::pGLDeleteTextures = nullptr;

PFNGLTEXTURESTORAGEMEM2DEXTPROC skq::SQGLFunctions::pGLTextureStorageMem2DEXT = nullptr;

void skq::SQGLFunctions::Initialize(QOpenGLContext* context) noexcept
{
    if (!pGLCreateMemoryObjectsEXT) 
        pGLCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)context->getProcAddress("glCreateMemoryObjectsEXT");
    q_check_ptr(pGLCreateMemoryObjectsEXT);

    if (!pGLDeleteMemoryObjectsEXT) 
        pGLDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)context->getProcAddress("glDeleteMemoryObjectsEXT");
    q_check_ptr(pGLDeleteMemoryObjectsEXT);

#ifdef _WIN32
    if (!pGLImportMemoryWin32NameEXT) 
        pGLImportMemoryWin32NameEXT = (PFNGLIMPORTMEMORYWIN32NAMEEXTPROC)context->getProcAddress("glImportMemoryWin32NameEXT");
    q_check_ptr(pGLImportMemoryWin32NameEXT);
#else
    if (!pGLImportMemoryFdEXT) 
        pGLImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC)context->getProcAddress("glImportMemoryFdEXT");
    q_check_ptr(pGLImportMemoryFdEXT);
#endif
    
    if (!pGLCreateTextures) 
        pGLCreateTextures = (PFNGLCREATETEXTURESPROC)context->getProcAddress("glCreateTextures");
    q_check_ptr(pGLCreateTextures);
    
    if (!pGLDeleteTextures) 
        pGLDeleteTextures = (PFNGLDELETETEXTURESEXTPROC)context->getProcAddress("glDeleteTexturesEXT");
    q_check_ptr(pGLDeleteTextures);
    
    if (!pGLTextureStorageMem2DEXT) 
        pGLTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)context->getProcAddress("glTextureStorageMem2DEXT");
    q_check_ptr(pGLTextureStorageMem2DEXT);
}