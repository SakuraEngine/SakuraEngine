#include "editor_core/sqgl_shared_resource.h"
#include <QtGui/qopenglext.h>

PFNGLCREATEMEMORYOBJECTSEXTPROC skq::SQGLFunctions::pGLCreateMemoryObjectsEXT = nullptr;
#ifdef _WIN32
PFNGLIMPORTMEMORYWIN32NAMEEXTPROC skq::SQGLFunctions::pGLImportMemoryWin32NameEXT = nullptr;
#else
PFNGLIMPORTMEMORYFDEXTPROC skq::SQGLFunctions::pGLImportMemoryFdEXT = nullptr;
#endif
PFNGLCREATETEXTURESPROC skq::SQGLFunctions::pGLCreateTextures = nullptr;
PFNGLTEXTURESTORAGEMEM2DEXTPROC skq::SQGLFunctions::pGLTextureStorageMem2DEXT = nullptr;

void skq::SQGLFunctions::Initialize(QOpenGLContext* context) noexcept
{
    if (!pGLCreateMemoryObjectsEXT) 
        pGLCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)context->getProcAddress("glCreateMemoryObjectsEXT");
    q_check_ptr(pGLCreateMemoryObjectsEXT);

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
    
    if (!pGLTextureStorageMem2DEXT) 
        pGLTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)context->getProcAddress("glTextureStorageMem2DEXT");
    q_check_ptr(pGLTextureStorageMem2DEXT);
}