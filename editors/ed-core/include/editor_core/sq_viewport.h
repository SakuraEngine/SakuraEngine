#pragma once
#include "edcore_configure.h"
#include "platform/process.h"
#include "lmdb/lmdb.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include "cgpu/api.h"
#include "sqgl_shared_resource.h"

namespace skq
{
class SKR_EDITOR_CORE_API SQViewport : public QOpenGLWidget
{
    Q_OBJECT
public:
    SQViewport(QWidget *partent, SProcessId provider) noexcept;
    ~SQViewport() noexcept;

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);  

private:
    struct ImportedTexture
    {
        GLuint memory_object = 0;
        GLuint gl_texture = 0;
    };
    ImportedTexture importTexture(uint64_t cgpu_handle, ECGPUBackend backend,
        uint32_t memorySize, uint32_t mipLevels, int oglFormat, uint32_t width, uint32_t height) noexcept;
    void deleteTexture(ImportedTexture& texture) noexcept;

    QOpenGLBuffer* gl_vb;           
    QOpenGLVertexArrayObject* gl_vao; 
    QOpenGLShaderProgram* gl_shader;  
    QOpenGLShaderProgram* gl_shader_with_tex;  

    SProcessId provider_id = UINT32_MAX;
    MDB_env* env = nullptr;
    MDB_dbi dbi = ~0;
    ImportedTexture imported_texture = {};
};
}