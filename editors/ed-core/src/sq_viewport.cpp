#include "editor_core/sq_viewport.h"
#include <QtGui/qopenglext.h>

skq::SQViewport::SQViewport(QWidget* parent) noexcept
    : QOpenGLWidget(parent), gl_vb(nullptr), gl_vao(nullptr), gl_shader(nullptr)
{

}

skq::SQViewport::~SQViewport() noexcept
{
    
}

static const char* VIEWPORT_VERTEX_SHADER_CODE =
    "#version 450\n"
    "layout(location = 0) in vec3 posVertex;\n"
    "layout(location = 1) in vec2 inUV;\n"
    "layout(location = 0) out vec2 outUV;\n"
    "void main() {\n"
    "  outUV = inUV;\n"
    "  gl_Position = vec4(posVertex, 1.0f);\n"
    "}\n";

static const char* VIEWPORT_FRAGMENT_SHADER_CODE =
    "#version 450\n"
    "layout (location = 0) in vec2 inUV;"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  fragColor = vec4(inUV, 0.0f, 1.0f);\n"
    "}\n";

void skq::SQViewport::initializeGL()
{
    gl_shader = new QOpenGLShaderProgram();
    gl_shader->addShaderFromSourceCode(QOpenGLShader::Vertex, VIEWPORT_VERTEX_SHADER_CODE);
    gl_shader->addShaderFromSourceCode(QOpenGLShader::Fragment, VIEWPORT_FRAGMENT_SHADER_CODE);
    if (gl_shader->link()) {
        qDebug("Shaders link success.");
    } else {
        qDebug("Shaders link failed!");
    }

    gl_vao = new QOpenGLVertexArrayObject();
    gl_vb = new QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);
    gl_vao->create();
    gl_vao->bind();
    
    static const GLfloat VERTEX_DATA[] = {
        -1.f, -1.f, 0.0f,
        1.f, -1.f, 0.0f,
        -1.f, 1.f, 0.0f,

        1.f, -1.f, 0.0f,
        1.f, 1.f, 0.0f,
        -1.f, 1.f, 0.0f, // Pos

        0.f, 0.f,
        1.f, 0.f,
        0.f, 1.f,

        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f // UV
    };
    gl_vb->create();
    gl_vb->bind();
    gl_vb->allocate(VERTEX_DATA, sizeof(VERTEX_DATA));
    SQGLFunctions::Initialize(this->context());

    QOpenGLFunctions *f = this->context()->functions();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    f->glEnableVertexAttribArray(1);
    f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)(sizeof(float) * 18));

    gl_vb->release();
    gl_vao->release();
}

skq::SQViewport::ImportedTexture skq::SQViewport::importTexture(
    uint64_t cgpu_handle, uint32_t memorySize, uint32_t mipLevels, int oglFormat, uint32_t width, uint32_t height)
{
    ImportedTexture output = {};
    const char* sharedName = nullptr;

    // Calc shared name with handle

    if (!sharedName) return output;
    // Create a 'memory object' in OpenGL, and associate it with the memory allocated in Vulkan
    SQGLFunctions::pGLCreateMemoryObjectsEXT(1, &output.memory_object);
#ifdef WIN32
    SQGLFunctions::pGLImportMemoryWin32NameEXT(output.memory_object, memorySize, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, sharedName);
#else
#endif
    SQGLFunctions::pGLCreateTextures(GL_TEXTURE_2D, 1, &output.gl_texture);
    SQGLFunctions::pGLTextureStorageMem2DEXT(output.gl_texture, mipLevels, oglFormat, width, height, output.memory_object, 0);
    return output;
}

void skq::SQViewport::paintGL()
{
    QOpenGLFunctions *f = this->context()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    f->glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
    gl_vao->bind();
    gl_shader->bind();
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    gl_shader->release();
    gl_vao->release();
}

void skq::SQViewport::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}