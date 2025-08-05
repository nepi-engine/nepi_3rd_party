#include "openglcontext.h"
#include "gltext.h"
#include "geometry.h"

#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QVector2D>
#include <QOpenGLFunctions>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QOpenGLContext>

#ifdef USE_GLES
#include <GLES3/gl3ext.h>
#else
#include <GL/gl.h>
#endif

#include <cmath>
#include "shaders.h"

FT_Library GLText::ms_ft = nullptr;
int GLText::ms_instanceCount = 0;

GLText::GLText()
:   m_program(0)
,   m_textureID(0)
,   m_textureUniform(0)
,   m_coordAttribute(0)
,   m_colourUniform(0)
,   m_mvMatrixUniform(0)
,   m_projMatrixUniform(0)
,   m_vbo(0)
,   m_face(nullptr)
,   m_fontsize(16)
,   m_colour (QColor(128, 128, 128))
,   m_offset(1)
,   m_monochrome(false)
,   m_allowCaching(false)
,   m_cachedGlyphTexture(0)
{
    ms_instanceCount++;

#ifndef NDEBUG
    // there's no need for it to have been called until we've been initialised
    m_cleanupCalled = true;
#endif
}

GLText::~GLText()
{
#ifndef NDEBUG
    Q_ASSERT(m_cleanupCalled);
#endif
    m_cachedGlyphMap.clear();
    m_freeGlyphSlots.clear();
    m_usedGlyphSlots.clear();

    if (m_face != nullptr)
    {
        FT_Done_Face(m_face);
        m_face = nullptr;
    }

    ms_instanceCount--;
    if (ms_instanceCount == 0)
    {
        if (ms_ft != nullptr)
        {
            FT_Done_FreeType(ms_ft);
            ms_ft = nullptr;
        }
    }
}

QString GLText::resolveFontPath(const QString& fontPath)
{
    QString filename = fontPath;

    QFile fontFile(filename);
    if (!fontFile.exists(filename))
    {
        QFileInfo fileInfo(filename);

        // check the application directory for the file
        filename = QCoreApplication::applicationDirPath() + "/fonts/" + fileInfo.fileName();
        if (!QFile::exists(filename))
        {
            // try and extract the file from the qrc resources
            QFile src(":/fonts/" + fileInfo.fileName());
            if (!src.open(QFile::ReadOnly))
            {
                // set back to the original file for returning out of this routine
                filename = fontPath;
            }
            else
            {
                // make sure the destination folder exists
                QDir dir(QFileInfo(filename).path());
                if (!dir.exists())
                {
                    dir.mkpath(dir.path());
                }

                QFile dst(filename);
                if (!dst.open(QFile::WriteOnly))
                {
                    // set back to the original file for returning out of this routine
                    filename = fontPath;
                }
                else
                {
                    dst.write(src.readAll());
                    dst.close();
                }
                src.close();
            }
        }
    }
    return filename;
}

void GLText::initialiseGL(QOpenGLContext& gl)
{
    QString filename = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/arial.ttf";
    initialiseGL(gl, filename);
#ifndef NDEBUG
    m_cleanupCalled = false;
#endif
}

GLuint GLText::compileProgram(const char* vertex, const char* fragment)
{  

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    if (vertShader)
    {
         glShaderSource(vertShader, 1, &vertex, nullptr);
         glCompileShader(vertShader);

        GLint success = 0;
         glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            // get error log from OpenGL
            char *log;
            GLint maxLength = 0;
             glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);
            log = static_cast<char*>(malloc(static_cast<size_t>(maxLength)));

             glGetShaderInfoLog(vertShader, maxLength, &maxLength, log);
            qDebug() << "Shader #" << vertShader << "Compile:" << log;
             glDeleteShader(vertShader); // Don't leak the shader.
        }
    }

    GLuint fragShader =  glCreateShader(GL_FRAGMENT_SHADER);
    if (fragShader)
    {
         glShaderSource(fragShader, 1, &fragment, nullptr);
         glCompileShader(fragShader);

        GLint success = 0;
         glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
        if(success == GL_FALSE)
        {
            // get error log from OpenGL
            char *log;
            GLint maxLength = 0;
             glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
            log = static_cast<char*>(malloc(static_cast<size_t>(maxLength)));

             glGetShaderInfoLog(fragShader, maxLength, &maxLength, log);
            qDebug() << "Shader #" << fragShader << "Compile:" << log;
             glDeleteShader(fragShader); // Don't leak the shader.
        }
    }

    // create the shader program
    GLuint program =  glCreateProgram();
     glAttachShader(program, vertShader);
     glAttachShader(program, fragShader);
     glLinkProgram(program);

    GLint status;
     glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        qDebug() << "program shader link failed";
        GLint infoLogLength;
         glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[static_cast<GLuint>(infoLogLength + 1)];
         glGetProgramInfoLog(program, infoLogLength, nullptr, strInfoLog);
        qDebug() << "Linker failure: " << strInfoLog;
        delete[] strInfoLog;
    }

    // clean up shaders
    if (vertShader)
    {
         glDetachShader(program, vertShader);
         glDeleteShader(vertShader);
    }
    if (fragShader)
    {
         glDetachShader(program, fragShader);
         glDeleteShader(fragShader);
    }

    return program;
}


void GLText::initialiseGL(QOpenGLContext& gl, const QString& fontPath)
{
    initializeOpenGLFunctions();

    // Initialize the FreeType2 library
    if (ms_ft == nullptr)
    {
        if (FT_Init_FreeType(&ms_ft))
        {
            qDebug() << "GLText ERROR: Could not initialise freetype library";
            return;
        }
    }

    QString filename = resolveFontPath(fontPath);

    QFileInfo info (filename);
    if (!info.exists())
    {
        qDebug() << "GLText ERROR: ttf does not exist" << fontPath;
        return;
    }

    if (m_face == nullptr)
    {
        int res = FT_New_Face(ms_ft, filename.toStdString().c_str(), 0, &m_face);
        if (res)
        {
            qDebug() << "GLText ERROR: could not create font face" << res << info.absolutePath();
            return;
        }
    }

    // Set default font size
    FT_Set_Pixel_Sizes(m_face, 0, m_fontsize);

#ifdef RASPI
    const char* vertex = (gl.isOpenGLES()) ? shaders::v320es::default_text_es_vert :  shaders::v110::default_text_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v320es::default_text_es_frag :  shaders::v110::default_text_frag;
#else
    const char* vertex = (gl.isOpenGLES()) ? shaders::v110es::default_text_es_vert : shaders::v110::default_text_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v110es::default_text_es_frag : shaders::v110::default_text_frag;
#endif

    m_program = compileProgram(vertex, fragment);


    if (m_program == 0)
    {
        qDebug() << "GLText ERROR: shader program failed:" << m_program;
        return;
    }

    glGenTextures(1, &m_textureID);
    setGLTexture(gl, m_textureID);

    m_coordAttribute = glGetAttribLocation(m_program, "coord");
    m_textureUniform = glGetUniformLocation(m_program, "tex");
    m_colourUniform = glGetUniformLocation(m_program, "color");

    if (m_coordAttribute == -1 || m_textureUniform == -1 || m_colourUniform == -1)
    {
        qDebug() << "GLText ERROR: Uniforms failed:" << m_coordAttribute << m_textureUniform << m_colourUniform;
        return;
    }

    // get shader matrix uniforms
    m_mvMatrixUniform = glGetUniformLocation(m_program, "modelViewMatrix");
    m_projMatrixUniform = glGetUniformLocation(m_program, "projMatrix");
    if (m_mvMatrixUniform == -1 || m_projMatrixUniform == -1)
    {
        qDebug() << "GLText ERROR: Matrix Uniforms failed:" << m_mvMatrixUniform << m_projMatrixUniform;
        return;
    }

    // Create the vertex buffer object
    glGenBuffers(1, &m_vbo);
    if (m_vbo == 0)
    {
        qDebug() << "GLText ERROR: VBO creation failed.";
        return;
    }

    if (m_allowCaching)
    {
        createGlyphCacheTexture(gl);
        generateGlyphSlots();
    }
}

/*
 * brief dispose of opengl resources
 */
void GLText::cleanup(QOpenGLContext& gl)
{
#ifndef NDEBUG
    m_cleanupCalled = true;
#endif
    if (m_textureID != 0)
    {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    if (m_cachedGlyphTexture != 0)
    {
        glDeleteTextures(1, &m_cachedGlyphTexture);
        m_cachedGlyphTexture = 0;
    }
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
}

/*
 * \brief sets up the texture for caching glyphs
 */
void GLText::createGlyphCacheTexture(QOpenGLContext& gl)
{
    // create the actual texture reference
    glGenTextures(1, &m_cachedGlyphTexture);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    setGLTexture(gl, m_cachedGlyphTexture);

    // create the actual texture and clear it to blank
    unsigned char* kBlankData = new unsigned char[kCachedTextureWidth * kCachedTextureHeight];
    memset(kBlankData, 0x0, kCachedTextureWidth * kCachedTextureHeight);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, kCachedTextureWidth, kCachedTextureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, kBlankData);
    delete [] kBlankData;

    glBindTexture(GL_TEXTURE_2D, 0);
}

/*
 * \brief sets up the slots on the texture page used for caching glyphs
 */
void GLText::generateGlyphSlots()
{
    // work out the size required to fill a slot on the texture page
    int maxGlyphWidth = 0;
    int maxGlyphHeight = 0;

    std::string standardChars = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{}|~";
    for (unsigned int n = 0; n < standardChars.size(); n++)
    {
        FT_GlyphSlot glyph = m_face->glyph;
        if (FT_Load_Char(m_face, (FT_ULong)standardChars[n], FT_LOAD_RENDER) == 0)
        {
            int width = glyph->bitmap.width;
            int height = glyph->bitmap.rows;
            maxGlyphWidth = (width > maxGlyphWidth) ? width : maxGlyphWidth;
            maxGlyphHeight = (height > maxGlyphHeight) ? height : maxGlyphHeight;

            QChar ch = QChar(standardChars[n]);
            m_cachedGlyphAdvance[ch.unicode()] = glyph->advance;
        }
    }

    // enumerate the available texture slots accordingly
    int slotsx = kCachedTextureWidth / maxGlyphWidth;
    int slotsy = kCachedTextureHeight / maxGlyphHeight;
    m_freeGlyphSlots.resize(slotsx * slotsy);

    int glyphSlot = 0;
    for (int y = 0; y < slotsy; y++)
    {
        for (int x = 0; x < slotsx; x++, glyphSlot++)
        {
            m_freeGlyphSlots[glyphSlot].u[0] = (float)(x * maxGlyphWidth) / kCachedTextureWidth;
            m_freeGlyphSlots[glyphSlot].v[0] = (float)(y * maxGlyphHeight) / kCachedTextureHeight;
            m_freeGlyphSlots[glyphSlot].loc.setX(x * maxGlyphWidth);
            m_freeGlyphSlots[glyphSlot].loc.setY(y * maxGlyphHeight);
            // width/height will be set on a per glyph basis as they come along
        }
    }
}

/*
 * \brief Attempts to load all characters onto the cached texture
 *        returns: 0 on failure
 *                 number of displayable characters on success
 */
unsigned int GLText::preloadGlyphs(QOpenGLContext& gl, const QString& text)
{
    if (!m_allowCaching)
        return false;

    unsigned int chrCount = 0;
    glBindTexture(GL_TEXTURE_2D, m_cachedGlyphTexture);

    for (int i = 0; i < text.size(); ++i)
    {
        QChar ch = text.at(i);
        if (ch != '\n')
        {
            chrCount++;

            FT_ULong chr = ch.unicode();
            if (m_cachedGlyphMap.count(chr))
            {
                // already in the cached texture, good to go
                continue;
            }
            GlyphData outGlyph;
            if (!cacheGlyph(gl, chr, outGlyph, m_cachedGlyphTexture))
            {
                return 0;
            }
        }
    }
    return chrCount;
}

/*
 * \brief provides the information required to draw a glyph that may already be locally cached
 *        NB: in the event of no cache space being available it will make use of "m_textureID"
 */
void GLText::getGlyphData(QOpenGLContext& gl, FT_ULong chr, GlyphData& outGlyph, GLuint& currentTexture)
{
    bool useCachedResult = false;
    if (m_allowCaching)
    {
        // attempt to use a cached version
        if (m_cachedGlyphMap.count(chr))
        {
            useCachedResult = true;
        }
        else if (cacheGlyph(gl, chr, outGlyph, currentTexture))
        {
            useCachedResult = true;
        }
        // ok, if this assertion has been hit we've run out of cached texture space and need to
        // decide whether or not to address the issue... it will/should(!) run fine in release
        // so this isn't critical but an error message alone will likely get ignored
        Q_ASSERT(useCachedResult);
    }

    if (useCachedResult)
    {
        outGlyph = m_cachedGlyphMap[chr];
        if (currentTexture != m_cachedGlyphTexture)
        {
            glBindTexture(GL_TEXTURE_2D, m_cachedGlyphTexture);
            currentTexture = m_cachedGlyphTexture;
        }
    }
    else
    {
        // bind to the basic texture we use for individual uploads
        if (currentTexture != m_textureID)
        {
            glBindTexture(GL_TEXTURE_2D, m_textureID);
            currentTexture = m_textureID;
        }

        FT_Int32 loadFlags = m_monochrome ? FT_LOAD_MONOCHROME | FT_LOAD_RENDER : FT_LOAD_RENDER;
        if (FT_Load_Char(m_face, chr, loadFlags) == 0)
        {
            FT_GlyphSlot glyph = m_face->glyph;
            if (m_cachedGlyphAdvance.find((ushort)chr) == m_cachedGlyphAdvance.end())
            {
                m_cachedGlyphAdvance[(ushort)chr] = glyph->advance;
            }

            // warn that we've run out of room on the texture... unless we get around to a LRU scheme for replacing glyphs
            if (m_monochrome)
            {
                unsigned char* pData = new unsigned char[glyph->bitmap.width * glyph->bitmap.rows];
                convertMonochromeData(pData, glyph);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph->bitmap.width, glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, pData);
                delete [] pData;
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, glyph->bitmap.width, glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
            }

            outGlyph.u[0] = 0;
            outGlyph.v[0] = 0;
            outGlyph.u[1] = 1;
            outGlyph.v[1] = 1;
            outGlyph.width = glyph->bitmap.width;
            outGlyph.height = glyph->bitmap.rows;
            outGlyph.bitmap_left = glyph->bitmap_left;
            outGlyph.bitmap_top = glyph->bitmap_top;
        }
        else
        {
            // error condition so just squish the uvs so as to display nothing
            outGlyph.u[0] = outGlyph.v[0] = outGlyph.u[1] = outGlyph.v[1] = 0;

            // and put in a nothing
            if (m_cachedGlyphAdvance.find((ushort)chr) == m_cachedGlyphAdvance.end())
            {
                FT_Vector advance;
                advance.x = advance.y = 0.0;
                m_cachedGlyphAdvance[(ushort)chr] = advance;
            }
        }
    }
}

/*
 * \brief copies data out of a glyph slot into a monchrome form we can render
 *        NB: no attempt at safe access of memory is made
 */
void GLText::convertMonochromeData(unsigned char* pData, FT_GlyphSlot glyph)
{
    for (int yy = 0; yy < glyph->bitmap.rows; yy++)
    {
        unsigned char* src = glyph->bitmap.buffer + (yy * glyph->bitmap.pitch);
        unsigned char* dst = pData + (yy * glyph->bitmap.width);

        unsigned char bitMask = 0x80;
        unsigned char srcByte = *src++;
        for (int xx = 0; xx < glyph->bitmap.width; xx++)
        {
            if ((srcByte & bitMask) != 0)
            {
                *dst++ = 0xff;
            }
            else
            {
                *dst++ = 0;
            }

            bitMask >>= 1;
            if (bitMask == 0)
            {
                srcByte = *src++;
                bitMask = 0x80;
            }
        }
    }
}

/*
 * \brief attempts to cache a glyph off to the texture page
 */
bool GLText::cacheGlyph(QOpenGLContext& gl, FT_ULong chr, GlyphData& outGlyph, GLuint& currentTexture)
{
    if (m_freeGlyphSlots.size() == 0)
    {
        return false;
    }

    FT_Int32 loadFlags = m_monochrome ? FT_LOAD_MONOCHROME | FT_LOAD_RENDER : FT_LOAD_RENDER;
    if (FT_Load_Char(m_face, chr, loadFlags) != 0)
    {
        return false;
    }

    if (currentTexture != m_cachedGlyphTexture)
    {
        glBindTexture(GL_TEXTURE_2D, m_cachedGlyphTexture);
        currentTexture = m_cachedGlyphTexture;
    }

    // extract the glyph data to use from the free slots
    outGlyph = m_freeGlyphSlots.back();
    m_freeGlyphSlots.pop_back();

    // transfer bitmap data to our image
    FT_GlyphSlot glyph = m_face->glyph;
    int glyphWidth = glyph->bitmap.width;
    int glyphHeight = glyph->bitmap.rows;
    if (m_monochrome)
    {
        unsigned char* pData = new unsigned char[glyphWidth * glyphHeight];
        convertMonochromeData(pData, glyph);
        glTexSubImage2D(GL_TEXTURE_2D, 0, outGlyph.loc.x(), outGlyph.loc.y(), glyphWidth, glyphHeight, GL_RED, GL_UNSIGNED_BYTE, pData);
        delete [] pData;
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, outGlyph.loc.x(), outGlyph.loc.y(), glyphWidth, glyphHeight, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
    }

    // update the portion of the glyph needing to be updated to the glyph used
    outGlyph.u[1] = outGlyph.u[0] + ((float)glyphWidth / kCachedTextureWidth);
    outGlyph.v[1] = outGlyph.v[0] + ((float)glyphHeight / kCachedTextureHeight);
    outGlyph.bitmap_left = glyph->bitmap_left;
    outGlyph.bitmap_top = glyph->bitmap_top;
    outGlyph.width = glyphWidth;
    outGlyph.height = glyphHeight;

    // assign to the used glyph slots which we could recover from with a LRU scheme
    m_usedGlyphSlots.push_back(outGlyph);

    // store in the map from character to glyph
    m_cachedGlyphMap[chr] = outGlyph;
    return true;
}

/*
 * \brief get width and height in terms of pixels of some spcified text. *
 */
void GLText::getWidthAndHeight(const QString& text, float scale, GLfloat &width, GLfloat &height)
{
    /*
    NB: The following might actually be the correct calculation...
    double pixel_size = m_fontsize;

    //Font height and width in pixels
    int font_height = round((face->bbox.yMax - face->bbox.yMin) * pixel_size / face->units_per_EM);

    height = font_height * scale;
    */
    height = m_fontsize * scale;
    width = 0.0;

    if (m_face)
    {
        for (int i = 0; i < text.size(); ++i)
        {
            ushort unicode = text.at(i).unicode();
            if (m_cachedGlyphAdvance.find(unicode) == m_cachedGlyphAdvance.end())
            {
                FT_GlyphSlot glyph = m_face->glyph;
                if (FT_Load_Char(m_face, unicode, FT_LOAD_DEFAULT))
                    continue;

                m_cachedGlyphAdvance[unicode] = FT_Vector(glyph->advance);
            }
            width += (m_cachedGlyphAdvance[unicode].x >> 6) * scale;
        }
    }
}


/*
 * \brief hand transforms text into 2D space, clamps this result and passes it through to the shader as is
 */
void GLText::draw2D(QOpenGLContext& gl, const QString& text, const QVector2D& pos, Qt::Alignment alignment, float scale)
{
    if (!m_program || !m_textureID)
        return;

    //glColor4f(0.0, 0.0, 0.0, 0.0);    //#See MS, removed for PI and still seems to work ok for Windows/Linux?
    glUseProgram(m_program);

    // Enable blending, necessary for our alpha texture
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Don't do depth testing on 2D text
    glDisable(GL_DEPTH_TEST);

    // Set text colour
    glUniform4f(m_colourUniform, m_colour.redF(), m_colour.greenF(), m_colour.blueF(), m_colour.alphaF());

    // no geometry transform is happening in 2D drawing but for now we're using the same shader as the 3D
    QMatrix4x4 modelView;
    modelView.setToIdentity();
    glUniformMatrix4fv(m_mvMatrixUniform, 1, GL_FALSE, modelView.constData());

    // construct the orthographic matrix for the 2D viewport, unambiguously determining viewport size
    float view[4];
    glGetFloatv(GL_VIEWPORT, view);
    QMatrix4x4 projectionMatrix;
    projectionMatrix.ortho(0, view[2], 0, view[3], -1.f, 1.f);
    glUniformMatrix4fv(m_projMatrixUniform, 1, GL_FALSE, projectionMatrix.constData());

    // Set up the VBO for our vertex data
    glEnableVertexAttribArray(m_coordAttribute);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(m_coordAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable texture settings
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_textureUniform, 0);

    // choose a texture to use
    GLuint currentTexture = m_allowCaching ? m_cachedGlyphTexture : m_textureID;
    glBindTexture(GL_TEXTURE_2D, currentTexture);

    // get the text size for alignment
    GLfloat width;
    GLfloat height;
    getWidthAndHeight(text, scale, width, height);

    FT_GlyphSlot glyph = m_face->glyph;

    // get origin position aligned accordingly
    GLfloat x = pos.x();
    GLfloat y = pos.y();
    if (alignment & Qt::AlignHCenter)
    {
        x -= width / 2.0f;
    }
    else if (alignment & Qt::AlignRight)
    {
        x -= width;
    }
    if (alignment & Qt::AlignTop)
    {
        y -= height;
    }
    else if (alignment & Qt::AlignVCenter)
    {
        y -= height / 2.0f;
    }

    // attempt to preload all the glyphs so we can batch draw the whole string
    unsigned int preloadCount = preloadGlyphs(gl, text);
    if (preloadCount != 0)
    {
        // NB: a successful run of preloadGl has already loaded up the cached texture
        std::vector<GLfloat> points;
        points.resize(preloadCount * 6 * 4);

        GLfloat xstart = x;
        unsigned int pointIndex = 0;
        for (int i = 0; i < text.size(); ++i)
        {
            QChar ch = text.at(i);
            if (ch == '\n')
            {
                // handle newline
                x = xstart;
                y -= height;
            }
            else
            {
                // load without rendering so as to get the advance
                if (FT_Load_Char(m_face, ch.unicode(), FT_LOAD_DEFAULT))
                    continue;

                GlyphData glyphData = m_cachedGlyphMap[ch.unicode()];

                // Calculate the vertex and texture coordinates and clamp to avoid sub-pixel inaccuracies
                float x2 = floor( x + (glyphData.bitmap_left * scale));
                float y2 = floor(-y - (glyphData.bitmap_top * scale));
                float w = glyphData.width * scale;
                float h = glyphData.height * scale;

                // tri 1
                points[pointIndex++] =  x2;
                points[pointIndex++] = -y2;
                points[pointIndex++] = glyphData.u[0];
                points[pointIndex++] = glyphData.v[0];

                points[pointIndex++] =  x2 + w;
                points[pointIndex++] = -y2;
                points[pointIndex++] = glyphData.u[1];
                points[pointIndex++] = glyphData.v[0];

                points[pointIndex++] =  x2;
                points[pointIndex++] = -y2 - h;
                points[pointIndex++] = glyphData.u[0];
                points[pointIndex++] = glyphData.v[1];

                // tri 2
                points[pointIndex++] =  x2;
                points[pointIndex++] = -y2 - h;
                points[pointIndex++] = glyphData.u[0];
                points[pointIndex++] = glyphData.v[1];

                points[pointIndex++] =  x2 + w;
                points[pointIndex++] = -y2;
                points[pointIndex++] = glyphData.u[1];
                points[pointIndex++] = glyphData.v[0];

                points[pointIndex++] =  x2 + w;
                points[pointIndex++] = -y2 - h;
                points[pointIndex++] = glyphData.u[1];
                points[pointIndex++] = glyphData.v[1];

                // Advance the cursor to the start of the next character
                x += (glyph->advance.x >> 6) * scale;
                y += (glyph->advance.y >> 6) * scale;
            }
        }
        // Draw the whole string to the screen
        glBufferData(GL_ARRAY_BUFFER, static_cast<int>(points.size() * sizeof(float)), &points[0], GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(6 * preloadCount));
    }
    else
    {
        // loop through all characters and draw
        GLfloat xstart = x;
        for (int i = 0; i < text.size(); ++i)
        {
            QChar ch = text.at(i);
            if (ch == '\n')
            {
                // handle newline
                x = xstart;
                y -= height;
            }
            else
            {
                // NB: getGlyphData may update the current texture in use
                GlyphData glyphData;
                getGlyphData(gl, ch.unicode(), glyphData, currentTexture);

                // Calculate the vertex and texture coordinates and clamp to avoid sub-pixel inaccuracies
                float x2 = floor( x + (glyphData.bitmap_left * scale));
                float y2 = floor(-y - (glyphData.bitmap_top * scale));
                float w = glyphData.width * scale;
                float h = glyphData.height * scale;

                GLfloat box[4][4] = {
                    {x2,     -y2    , glyphData.u[0], glyphData.v[0]},
                    {x2 + w, -y2    , glyphData.u[1], glyphData.v[0]},
                    {x2,     -y2 - h, glyphData.u[0], glyphData.v[1]},
                    {x2 + w, -y2 - h, glyphData.u[1], glyphData.v[1]},
                };

                // Draw the character on the screen
                glBufferData(GL_ARRAY_BUFFER, sizeof(box), &box[0], GL_DYNAMIC_DRAW);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                // Advance the cursor to the start of the next character
                x += (glyph->advance.x >> 6) * scale;
                y += (glyph->advance.y >> 6) * scale;
            }
        }
    }

    glDisableVertexAttribArray(m_coordAttribute);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

/*
 * \brief Avoids repeating the settings required to setup a texture for drawing
 * NOTE: GL_CLAMP was changed to GL_CLAMP_TO_EDGE after a) Rasp PI not recognising it (different version) and 2) reading on various GL forums that
 * GL_CLAMP_TO_EDGE is the preferred option.
 */
void GLText::setGLTexture(QOpenGLContext& gl, GLuint textureID)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    if (m_monochrome)
    {
        // small text that's axis aligned can look much sharper using monochrome without filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else
    {
        // Linear filtering usually looks best for text
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    // Clamping to edges is important to prevent artifacts when scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // We require 1 byte alignment when uploading texture data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void GLText::setFontSize(unsigned int fontsize)
{
    m_fontsize = fontsize;
}

void GLText::setColour(const QColor& colour)
{
    m_colour = colour;
}

void GLText::setAllowCaching(bool b)
{
    m_allowCaching = b;
}
