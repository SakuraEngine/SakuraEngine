#include "editor_core/sq_viewport.h"
#include <QtGui/qopenglext.h>

#include "utils/log.h"

inline static void env_create(MDB_env** penv)
{
    if (const int rc = mdb_env_create(penv)) 
    {
        SKR_LOG_ERROR("mdb_env_create failed: %d", rc);
    }
    else
    {
        SKR_LOG_INFO("env_create succeed: %d", rc);
    }
    mdb_env_set_maxdbs(*penv, 50);
    mdb_env_set_mapsize(*penv, (size_t)1048576 * (size_t)16); // 1MB * 16
    if (const int rc = mdb_env_open(*penv, "./cross-proc", 0, 0664)) 
    {
        SKR_LOG_ERROR("mdb_env_open failed: %d", rc);
        env_create(penv);
    }
    else
    {
        SKR_LOG_INFO("mdb_env_open succeed: %d", rc);
    }
}

inline static void dbi_create(MDB_env* env, MDB_dbi* pdb, bool readonly)
{
    // Open txn
    MDB_txn* txn;
    if (const int rc = mdb_txn_begin(env, nullptr, readonly ? MDB_RDONLY : 0, &txn)) 
    {
        SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
    }
    else
    {
        SKR_LOG_INFO("mdb_txn_begin succeed: %d", rc);
    }
    // Txn body: open db
    {
        // ZoneScopedN("MDBQuery");

        auto dbi_flags = MDB_CREATE;
        if (const int rc = mdb_dbi_open(txn, "proc-links", dbi_flags, pdb)) 
        {
            SKR_LOG_ERROR("mdb_dbi_open failed: %d", rc);
        }
        else if (!readonly)
        {
            if (const int rc = mdb_drop(txn, *pdb, 0) )
            {
                SKR_LOG_ERROR("mdb_dbi_drop failed: %d", rc);
            }
        }
    }
    mdb_txn_commit(txn);
}

CGPUImportTextureDescriptor receiver_get_shared_handle(MDB_env* env, MDB_dbi dbi, SProcessId provider_id)
{
    CGPUImportTextureDescriptor what = {};
    what.shared_handle = UINT64_MAX;
    if (dbi == ~0) return what;
    MDB_txn* txn = nullptr;
    if (const int rc = mdb_txn_begin(env, nullptr, MDB_RDONLY, &txn)) 
    {
        SKR_LOG_ERROR("mdb_txn_begin failed: %d", rc);
    }
    MDB_cursor *cursor;
    if (int rc = mdb_cursor_open(txn, dbi, &cursor)) 
    {
        SKR_LOG_ERROR("mdb_cursor_open failed: %d", rc);
    }

    //Initialize the key with the key we're looking for
    eastl::string keyString = eastl::to_string(provider_id);
    MDB_val key = { (size_t)keyString.size(), (void*)keyString.data() };
    MDB_val data;

    //Position the cursor, key and data are available in key
    if (int rc = mdb_cursor_get(cursor, &key, &data, MDB_SET_KEY)) 
    {
        //No value found
        SKR_LOG_TRACE("query proc-links with key %s found no value: %d", keyString.c_str(), rc);
        mdb_cursor_close(cursor);
    }
    else
    {
        what = *(CGPUImportTextureDescriptor*)data.mv_data;
        mdb_cursor_close(cursor);
    }
    mdb_txn_commit(txn);
    return what;
}

skq::SQViewport::SQViewport(QWidget* parent, SProcessId provider) noexcept
    : QOpenGLWidget(parent), gl_vb(nullptr), gl_vao(nullptr), gl_shader(nullptr), provider_id(provider)
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
    "  fragColor = vec4(0.35f, 0.35f, 0.35f, 1.0f);\n"
    "}\n";

static const char* VIEWPORT_WITH_TEXTURE_FRAGMENT_SHADER_CODE =
    "#version 450\n"
    "layout (location = 0) in vec2 inUV;"
    "uniform sampler2D color_texture;"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "  fragColor = texture(color_texture, inUV);\n"
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

    gl_shader_with_tex = new QOpenGLShaderProgram();
    gl_shader_with_tex->addShaderFromSourceCode(QOpenGLShader::Vertex, VIEWPORT_VERTEX_SHADER_CODE);
    gl_shader_with_tex->addShaderFromSourceCode(QOpenGLShader::Fragment, VIEWPORT_WITH_TEXTURE_FRAGMENT_SHADER_CODE);
    if (gl_shader_with_tex->link()) {
        qDebug("Shaders link success.");
        QOpenGLFunctions *f = this->context()->functions();
        gl_shader_with_tex->bind();
        f->glUniform1i(gl_shader_with_tex->uniformLocation("color_texture"), 0);
        gl_shader_with_tex->release();
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

        0.f, 1.f,
        1.f, 1.f,
        0.f, 0.f,

        1.f, 1.f,
        1.f, 0.f,
        0.f, 0.f // UV
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

    env_create(&env);
}

skq::SQViewport::ImportedTexture skq::SQViewport::importTexture(uint64_t cgpu_handle, ECGPUBackend backend,
    uint32_t memorySize, uint32_t mipLevels, int oglFormat, uint32_t width, uint32_t height) noexcept
{
    ImportedTexture output = {};
    const eastl::wstring shared_texture_name_format = L"cgpu-shared-texture-";
    eastl::wstring name = shared_texture_name_format;

    // Calc shared name with handle
    if (cgpu_handle != UINT64_MAX)
    {
        name += eastl::to_wstring(cgpu_handle);
    }
    else
    {
        return output;
    }

    // Create a 'memory object' in OpenGL, and associate it with the memory allocated in Vulkan
    SQGLFunctions::pGLCreateMemoryObjectsEXT(1, &output.memory_object);
#ifdef WIN32
    const auto glHandleType = backend == CGPU_BACKEND_D3D12 ? GL_HANDLE_TYPE_D3D12_RESOURCE_EXT : GL_HANDLE_TYPE_OPAQUE_WIN32_EXT;
    SQGLFunctions::pGLImportMemoryWin32NameEXT(output.memory_object, memorySize, glHandleType, name.c_str());
#else
#endif
    SQGLFunctions::pGLCreateTextures(GL_TEXTURE_2D, 1, &output.gl_texture);
    SQGLFunctions::pGLTextureStorageMem2DEXT(output.gl_texture, mipLevels, oglFormat, width, height, output.memory_object, 0);
    QOpenGLFunctions *f = this->context()->functions();
    f->glTexParameteri(output.gl_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    f->glTexParameteri(output.gl_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    f->glTexParameteri(output.gl_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    f->glTexParameteri(output.gl_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return output;
}

void skq::SQViewport::deleteTexture(skq::SQViewport::ImportedTexture& texture) noexcept
{
    if (SQGLFunctions::pGLDeleteTextures) SQGLFunctions::pGLDeleteTextures(1, &texture.gl_texture);
    if (SQGLFunctions::pGLDeleteMemoryObjectsEXT) SQGLFunctions::pGLDeleteMemoryObjectsEXT(1, &texture.memory_object);
    texture.memory_object = 0;
    texture.gl_texture = 0;
}

void skq::SQViewport::paintGL()
{
    if (dbi == ~0) 
    {
        dbi_create(env, &dbi, true);
    }
    if (dbi != ~0)
    {
        static uint64_t cached_shared_handle = UINT64_MAX;
        if (auto imported_info = receiver_get_shared_handle(env, dbi, provider_id); 
            cached_shared_handle != imported_info.shared_handle && imported_info.shared_handle != UINT64_MAX)
        {
            SKR_LOG_DEBUG("Receiver try to import shared texture with handle %llu", imported_info.shared_handle);
            if (imported_texture.gl_texture != 0)
            {
                deleteTexture(imported_texture);
            }
            imported_texture = importTexture(
                imported_info.shared_handle, imported_info.backend,
                imported_info.size_in_bytes, imported_info.mip_levels, 
                /*imported_info.format*/GL_RGBA8, imported_info.width, imported_info.height);
        }
    }
    QOpenGLFunctions *f = this->context()->functions();
    f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    f->glClearColor(0.0f, 0.2f, 0.2f, 1.0f);
    gl_vao->bind();
    if (imported_texture.gl_texture == 0)
    {
        gl_shader->bind();
    }
    else
    {
        gl_shader_with_tex->bind();
        // set the texture wrapping parameters
        f->glActiveTexture(GL_TEXTURE0);
        f->glBindTexture(GL_TEXTURE_2D, imported_texture.gl_texture);
    }
    f->glDrawArrays(GL_TRIANGLES, 0, 6);
    if (imported_texture.gl_texture == 0)
    {
        gl_shader->release();
    }
    else
    {
        gl_shader_with_tex->release();
    }
    gl_vao->release();
}

void skq::SQViewport::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}