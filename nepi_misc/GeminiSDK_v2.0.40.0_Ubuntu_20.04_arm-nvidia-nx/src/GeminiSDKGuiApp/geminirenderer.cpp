#include "geminirenderer.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QSurfaceFormat>
#include <QtGui/QWheelEvent>

#ifdef USE_GLES
#include <GLES3/gl3ext.h>
#else
#include <GL/gl.h>
#endif

//#define FIRE2_PALETTE

#ifdef FIRE2_PALETTE
#include "fire2palette.h"
#else
#include "sonarpalette.h"
#include "spectrumpalette.h"
#endif
#include "shaders.h"

#include <math.h>

GeminiRenderer::GeminiRenderer()
:   m_program(0)
,   m_paletteID(0)
,   m_textureID(0)
,   m_paletteUniform(-1)
,   m_textureUniform(-1)
,   m_matrixUniform(-1)
,   m_bearings(0)
,   m_aspectRatio(1.0f)
{
}

void GeminiRenderer::initialiseGL(QOpenGLContext &gl)
{
    initializeOpenGLFunctions();


#ifdef RASPI
    const char* vertex = (gl.isOpenGLES()) ? shaders::v320es::sonar_es_vert : shaders::v110::sonar_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v320es::sonar_es_frag : shaders::v110::sonar_frag;
#else
    const char* vertex = (gl.isOpenGLES()) ? shaders::v100es::sonar_es_vert : shaders::v110::sonar_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v100es::sonar_es_frag : shaders::v110::sonar_frag;
#endif

    m_program = compileProgram(vertex, fragment);


    if (0 != m_program)
    {
        glGenTextures(1, &m_textureID);

        // get shader texture uniforms
        m_paletteUniform = glGetUniformLocation(m_program, "paletteTexture");
        if (m_paletteUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: paletteTexture";
        }

        m_textureUniform = glGetUniformLocation(m_program, "sonarTexture");
        if (m_textureUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: sonarTexture";
        }

        m_matrixUniform = glGetUniformLocation(m_program, "matrix");
        if (m_matrixUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: matrix";
        }

        initPalette();
    }

#ifdef RASPI
    vertex = (gl.isOpenGLES()) ? shaders::v320es::text_es_vert : shaders::v110::text_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v320es::text_es_frag : shaders::v110::text_frag;
#else
    vertex = (gl.isOpenGLES()) ? shaders::v100es::text_es_vert : shaders::v110::text_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v100es::text_es_frag : shaders::v110::text_frag;
#endif


    m_text.initialiseGL(compileProgram(vertex, fragment));

#ifdef RASPI
    vertex = (gl.isOpenGLES()) ? shaders::v320es::default_es_vert : shaders::v110::default_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v320es::default_es_frag : shaders::v110::default_frag;
#else
    vertex = (gl.isOpenGLES()) ? shaders::v100es::default_es_vert : shaders::v110::default_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v100es::default_es_frag : shaders::v110::default_frag;
#endif

    m_grid.initialiseGL(compileProgram(vertex, fragment));
}

double GeminiRenderer::CalculateRangeInMeters(double rangeLines, bool glfLogger)
{
    if (glfLogger)
    {
        return ( rangeLines * (m_image.m_mainImage.m_fSosAtXd / 2.0) / m_image.m_mainImage.m_uiModulationFrequency );
    }
    else
    {
#ifdef _ECD_LOGGER_
        return ( rangeLines * (m_ecdImage.m_clTgtRec.m_clPing.m_fSosAtXd / 2.0) / m_ecdImage.m_clTgtRec.m_clPing.m_iModulationFrequency );
#endif
    }
    return 0;
}

void GeminiRenderer::paintGL(QOpenGLContext& gl, float zoom, int width, int height, bool glfLogger, bool drawText, bool drawGrid)
{
    if (0 == m_program)
        return;         // initialisation has failed
    if (0 == m_vertices.size())
        return; // no valid image to draw

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // fit sonar data into the view
    float aspectRatio = (0 != height) ? width/ (float)height : 1.0f;

    float cameraZoom = 1.0 / (zoom * zoom);
    if (m_aspectRatio > aspectRatio && 0.0 != aspectRatio)
    {
        cameraZoom *= m_aspectRatio / aspectRatio;
    }

    QMatrix4x4 camera;
    camera.lookAt(QVector3D(0, 0, 1.3 * cameraZoom), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    camera.translate(0.0f, -0.5f, 0.0f);
    camera.scale(1.0f, 1.0f, 1.0f);

    QMatrix4x4 projection;
    projection.setToIdentity();
    projection.perspective(45.0f, (float)width / (float)height, 0.01f, 100.0f);

    QMatrix4x4 m = projection * camera;
    paintSonarImage(m.constData(), glfLogger);

    double start, end, range;
    if (glfLogger)
    {
        // get start/end angles from the bearing table
        std::vector<double>& bearingTbl = *m_image.m_mainImage.m_vecBearingTable;
        if (!bearingTbl.size())
        {
            return;
        }
        start = bearingTbl[m_image.m_mainImage.m_uiStartBearing] * 180.0 / M_PI;
        end = bearingTbl[m_image.m_mainImage.m_uiEndBearing - 1] * 180.0 / M_PI;
        range = CalculateRangeInMeters(m_image.m_mainImage.m_uiEndRange, glfLogger);
    }
    else
    {
#ifdef _ECD_LOGGER_
        const CBeamInfo& info = m_ecdImage.m_clTgtRec.m_clPing.m_clBeamInfo;
        if( !info.m_pBearingTable )
        {
            return;
        }
        start = info.m_pBearingTable[0] * 180.0 / M_PI;
        end = info.m_pBearingTable[m_ecdImage.m_uiNumBearings - 1] * 180.0 / M_PI;
        range = CalculateRangeInMeters(m_ecdImage.m_uiNumRanges, glfLogger);
#endif
    }

    if (m_range != range)
    {
        m_range = range;

        m_ranges.clear();
        getRangeLabels(m_range, m_ranges);        
    }

    // draw the text labels
    if (drawText)
    {
        int size = (width < height) ? width : height;
        m_text.paintGL(m, size, m_range, m_ranges, start, end, zoom);
    }
    if (drawGrid)
    {
        m_grid.paintGL(m, m_range, m_ranges, start, end);
    }

    //drawOfflineImage(glfLogger);
}

void GeminiRenderer::paintSonarImage(const float* mvp, bool glfLogger)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(m_program);

    // bind matrix
    glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, mvp);

    // bind palette
    glUniform1i(m_paletteUniform, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_paletteID);

    // bind sonar image texture and update
    glUniform1i(m_textureUniform, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (glfLogger)
    {
        std::vector<UInt8>&vecData = *m_image.m_mainImage.m_vecData;
        if( vecData.size() )
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
                (m_image.m_mainImage.m_uiEndBearing - m_image.m_mainImage.m_uiStartBearing),
                m_image.m_mainImage.m_uiEndRange, 0,
                GL_RED, GL_UNSIGNED_BYTE, &vecData[0]);
        }                
    }
    else
    {
#ifdef _ECD_LOGGER_
        if( m_ecdImage.m_pData )
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                m_ecdImage.m_uiNumBearings, m_ecdImage.m_uiNumRanges, 0,
                GL_RED, GL_UNSIGNED_BYTE, m_ecdImage.m_pData);
        }        
#endif
    }

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        s_elementSize * sizeof(GLfloat), &m_vertices[0]);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        s_elementSize * sizeof(GLfloat), &m_vertices[2]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_vertices.size() / s_elementSize);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);    // release program
}

void GeminiRenderer::initPalette()
{
    if (0 == m_paletteID)
    {
        glGenTextures(1, &m_paletteID); // create a texture ID handle
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_paletteID); // bind the texture

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_KEEP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_KEEP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#ifdef FIRE2_PALETTE
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &palette::fire2[0]);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 1, 0,
             GL_RGB, GL_UNSIGNED_BYTE, &palette::spectrum[0]);

#endif
    glBindTexture(GL_TEXTURE_2D, 0); //unbind
}

GLuint GeminiRenderer::compileProgram(const char* vertex, const char* fragment)
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
            log = (char*)malloc(maxLength);

            glGetShaderInfoLog(vertShader, maxLength, &maxLength, log);
            qDebug() << "Shader #" << vertShader << "Compile:" << log;
            glDeleteShader(vertShader); // Don't leak the shader.
        }
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
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
            log = (char*)malloc(maxLength);

            glGetShaderInfoLog(fragShader, maxLength, &maxLength, log);
            qDebug() << "Shader #" << fragShader << "Compile:" << log;
            glDeleteShader(fragShader); // Don't leak the shader.
        }
    }

    // create the shader program
    GLuint program = glCreateProgram();
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

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
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

void GeminiRenderer::cleanup(QOpenGLContext& gl)
{
    glDeleteTextures(1, &m_textureID);
    m_textureID = 0;
    glDeleteTextures(1, &m_paletteID);
    m_paletteID = 0;
    glDeleteShader(m_program);
    m_program = 0;
}

bool GeminiRenderer::hasRangeChanged(const GLF::GLogTargetImage& image, double *new_range)
{
    *new_range = CalculateRangeInMeters(image.m_mainImage.m_uiEndRange, true);

    if(m_range != *new_range) {
        return true;
    }
    else {
        return false;
    }
}

#ifdef _ECD_LOGGER_
bool GeminiRenderer::hasRangeChanged(const CTgtImg& image, double *new_range)
{
    *new_range = CalculateRangeInMeters(image.m_uiNumRanges, false);

    if(m_range != *new_range) {
        return true;
    }
    else {
        return false;
    }
}
#endif

void GeminiRenderer::setVertices(const GLF::GLogTargetImage& image)
{
    m_image = image;

    unsigned int numBearings = m_image.m_mainImage.m_uiEndBearing;
    std::vector<double>& bearingTbl = *m_image.m_mainImage.m_vecBearingTable;
    // skip invalid sonar data
    // verify the beam table matches the image size
    if (0 == m_image.m_mainImage.m_uiEndBearing ||
        0 == m_image.m_mainImage.m_uiEndRange   ||
        0 == m_image.m_mainImage.m_vecData->size() )
    {
        m_vertices.resize(0);
        return;
    }

    // bearing lines * vertex element size * (top+bottom)
    unsigned int numVertices = (numBearings) * GeminiRenderer::s_elementSize * 2;

    // skip if the geometry hasn't changed
    if (numBearings == m_bearings &&
        m_vertices.size() == numVertices)
    {
        return;
    }

    // initialise the vertex buffer
    m_vertices.resize(numVertices, 0);
    m_bearings = numBearings;

    int v = 0;
    for (unsigned int b = 0; b < m_bearings; ++b)
    {
        // convert into a rotation from M_PI_2
        double bearing = bearingTbl[b] + M_PI_2;

        m_vertices[v++] = (float)(cos(bearing) * 1.0);      // *Tip: change 1.0 to range for real coordinate system
        m_vertices[v++] = (float)(sin(bearing) * 1.0);      // *Tip: change 1.0 to range for real coordinate system
        m_vertices[v++] = b / (float)m_bearings;            // S texture coordinate
        m_vertices[v++] = 1.0f;                             // T texture coordinate

        m_vertices[v++] = 0.0f;                             // start range 0.0
        m_vertices[v++] = 0.0f;                             // start range 0.0
        m_vertices[v++] = b / (float)m_bearings;            // S texture coordinate
        m_vertices[v++] = 0.0f;                             // T texture coordinate
    }

    // calculate the sonar image aspect ratio
    if (m_bearings)
    {
        // height is 1.0 as above so calculate width as ratio
        float start = (float)(bearingTbl[0] + M_PI_2);
        float end = (float)(bearingTbl[m_bearings - 1] + M_PI_2);
        m_aspectRatio = fabs(cos(end) - cos(start));
    }
    else
    {
        m_aspectRatio = 1.0f;
    }
}

#ifdef _ECD_LOGGER_
void GeminiRenderer::setVertices(const CTgtImg& image)
{
    // Update image locally
    m_ecdImage = image;

    const CBeamInfo& info = image.m_clTgtRec.m_clPing.m_clBeamInfo;

    // skip invalid sonar data
    // verify the beam table matches the image size
    if (0 == image.m_uiNumBearings ||
        0 == image.m_uiNumRanges ||
        image.m_uiNumBearings != info.m_sNumBeams ||
        !image.m_pData)
    {
        m_vertices.resize(0);
        return;
    }

    // bearing lines * vertex element size * (top+bottom)
    unsigned int numVertices = (image.m_uiNumBearings) * GeminiRenderer::s_elementSize * 2;

    // skip if the geometry hasn't changed
    if (image.m_uiNumBearings == m_bearings &&
        m_vertices.size() == numVertices)
    {
        return;
    }

    // initialise the vertex buffer
    m_vertices.resize(numVertices, 0);
    m_bearings = image.m_uiNumBearings;

    int v = 0;
    for (unsigned int b = 0; b < m_bearings; ++b)
    {
        // convert into a rotation from M_PI_2
        double bearing = info.m_pBearingTable[b] + M_PI_2;

        m_vertices[v++] = (float)(cos(bearing) * 1.0);      // *Tip: change 1.0 to range for real coordinate system
        m_vertices[v++] = (float)(sin(bearing) * 1.0);      // *Tip: change 1.0 to range for real coordinate system
        m_vertices[v++] = b / (float)m_bearings;            // S texture coordinate
        m_vertices[v++] = 1.0f;                             // T texture coordinate

        m_vertices[v++] = 0.0f;                             // start range 0.0
        m_vertices[v++] = 0.0f;                             // start range 0.0
        m_vertices[v++] = b / (float)m_bearings;            // S texture coordinate
        m_vertices[v++] = 0.0f;                             // T texture coordinate
    }

    // calculate the sonar image aspect ratio
    if (m_bearings)
    {
        // height is 1.0 as above so calculate width as ratio
        float start = (float)(info.m_pBearingTable[0] + M_PI_2);
        float end = (float)(info.m_pBearingTable[m_bearings - 1] + M_PI_2);
        m_aspectRatio = fabs(cos(end) - cos(start));
    }
    else
    {
        m_aspectRatio = 1.0f;
    }
}
#endif

void GeminiRenderer::getRangeLabels(double range, std::vector<double>& ranges) const
{
    double marker = range / 4.0;

    std::vector<double> markers;
    markers.push_back(0.1);
    markers.push_back(0.15);
    markers.push_back(0.2);
    markers.push_back(0.25);
    markers.push_back(0.5);
    markers.push_back(0.75);

    bool found = false;
    unsigned int pos = 0;
    double ratio = 0.1;

    while (marker >= markers[markers.size()-1] * ratio)
    {
        ratio *= 10.0;
    }

    double last = markers[markers.size()-1] * ratio / 10.0;

    while (!found && pos < markers.size())
    {
        double next = markers[pos] * ratio;

        if (marker < next)
        {
            if (fabs(marker - last) < fabs(marker - next))
            {
                marker = last;
            }
            else
            {
                marker = next;
            }
            found  = true;
        }

        last = next;
        pos++;
    }

    if (0.0 == marker)
    {
        marker = 1.0;
    }

    ranges.clear();
    unsigned int count = (range * 0.95) / marker;
    for (unsigned int i = 1; i <= count; ++i)
    {
        ranges.push_back(marker * i);
    }

    ranges.push_back(range);
}

bool GeminiRenderer::saveXYToBMP(int w, int h, std::string& filename, bool glfLogger)
{
    if (filename.length() < 5)
    {
        // expecting at least "1.bmp", for example
        return false;
    }

    // TODO: test for/create missing directories
    if (FILE* file = fopen(filename.c_str(), "r"))
    {
        // don't overwrite an existing file
        fclose(file);
        return false;
    }

    if ((0 == m_program) || (0 == m_vertices.size()))
    {
        // initialisation has failed, or no valid image to draw
        return false;
    }

    if (glfLogger)
    {
        // skip invalid sonar data
        // verify the beam table matches the image size
        if (0 == m_image.m_mainImage.m_uiEndBearing ||
            0 == m_image.m_mainImage.m_uiEndRange   ||
            0 == m_image.m_mainImage.m_vecData->size() )
        {
            // skip invalid sonar data
            return false;
        }
    }
    else
    {
#ifdef _ECD_LOGGER_
        const CBeamInfo& info = m_ecdImage.m_clTgtRec.m_clPing.m_clBeamInfo;
        if ((0 == m_ecdImage.m_uiNumBearings) ||
            (0 == m_ecdImage.m_uiNumRanges) ||
            (m_ecdImage.m_uiNumBearings != info.m_sNumBeams) ||
            !m_ecdImage.m_pData)
#endif
        {
            // skip invalid sonar data
            return false;
        }
    }

    // get state that needs restoring
    float viewport[4];
    glGetFloatv(GL_VIEWPORT, viewport);

    unsigned int fboTexture;
    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, w, h);

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    //glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glViewport(0, 0, w, h);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float startBearing, endBearing, midBearing;
    int numBearing;

    if (glfLogger)
    {
        std::vector<double>& bearingTbl = *m_image.m_mainImage.m_vecBearingTable;
        startBearing = bearingTbl[0] + M_PI_2;
        numBearing = m_image.m_mainImage.m_uiEndBearing - m_image.m_mainImage.m_uiStartBearing;
        endBearing = bearingTbl[numBearing - 1] + M_PI_2;
        midBearing = bearingTbl[(numBearing / 2) - 1] + M_PI_2;
    }
    else
    {
#ifdef _ECD_LOGGER_
        const CBeamInfo& info = m_ecdImage.m_clTgtRec.m_clPing.m_clBeamInfo;
        if( !info.m_pBearingTable )
        {
            return false;
        }
        startBearing = info.m_pBearingTable[0] + M_PI_2;
        numBearing =  m_ecdImage.m_uiNumBearings;
        endBearing = info.m_pBearingTable[numBearing - 1] + M_PI_2;
        midBearing = info.m_pBearingTable[(numBearing / 2) - 1] + M_PI_2;
#endif
    }

    float minx = cos(startBearing);
    float maxx = cos(endBearing);
    float maxy = sin(midBearing);
    float miny = 0.f;
    float farPlane = 1.f;
    float nearPlane = -1.f;

    float transformMatrix[16];
    memset(transformMatrix, 0, sizeof(float) * 16);

    // setup an orthographic projection matrix translated along the x-axis for the camera's position
    transformMatrix[0] = 2.f / (maxx - minx);
    transformMatrix[5] = 2.f / (maxy - miny);
    transformMatrix[10] = -2.f / (farPlane - nearPlane);
    transformMatrix[15] = 1.f;

    transformMatrix[12] = (maxx + minx) / (maxx - minx);
    transformMatrix[13] = (maxy + miny) / (maxy - miny);
    transformMatrix[14] = (farPlane + nearPlane) / (farPlane - nearPlane);

    // the effect of multiplying by a translation matrix offset by -1.f along the z-axis
    transformMatrix[13] = -1.f;
    transformMatrix[14] =  1.f;

    paintSonarImage(transformMatrix, glfLogger);

    unsigned char* screen = (unsigned char*)malloc(w * h * 3);
    glReadPixels(0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, screen);
    bool saveBMPResult = SaveBMP(w, h, screen, filename.c_str());
    free(screen);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteTextures(1, &fboTexture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);

    // reset viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    return saveBMPResult;
}


bool GeminiRenderer::SaveBMP(const int width, const int height, const unsigned char* pixels, const char* path)
{
    unsigned char bmp_file_header[14] = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, };
    unsigned char bmp_info_header[40] = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, };
    unsigned char bmp_pad[3] = { 0, 0, 0, };

    const int size = 54 + width * height * 3;

    bmp_file_header[2] = static_cast<unsigned char>(size);
    bmp_file_header[3] = static_cast<unsigned char>(size >> 8);
    bmp_file_header[4] = static_cast<unsigned char>(size >> 16);
    bmp_file_header[5] = static_cast<unsigned char>(size >> 24);

    bmp_info_header[4] = static_cast<unsigned char>(width);
    bmp_info_header[5] = static_cast<unsigned char>(width >> 8);
    bmp_info_header[6] = static_cast<unsigned char>(width >> 16);
    bmp_info_header[7] = static_cast<unsigned char>(width >> 24);

    bmp_info_header[8] = static_cast<unsigned char>(height);
    bmp_info_header[9] = static_cast<unsigned char>(height >> 8);
    bmp_info_header[10] = static_cast<unsigned char>(height >> 16);
    bmp_info_header[11] = static_cast<unsigned char>(height >> 24);

    FILE* file = fopen(path, "wb");
    if (file)
    {
        fwrite(bmp_file_header, 1, 14, file);
        fwrite(bmp_info_header, 1, 40, file);

        for (int i = 0; i < height; i++)
        {
            fwrite(pixels + (width * i * 3), 3, width, file);
            fwrite(bmp_pad, 1, ((4 - (width * 3) % 4) % 4), file);
        }
        fclose(file);
        return true;
    }
    return false;
}

void GeminiRenderer::drawOfflineImage(/*QOpenGLContext& gl, */bool glfLogger)
{
    int frameWidth = 1080;
    int frameHeight = floor(frameWidth / m_aspectRatio);

    std::string fileName = "C:\\Frame Capture\\image.bmp";
    saveXYToBMP(frameWidth, frameHeight, fileName, glfLogger);
    /*
    GLRenderers& renderers = getRenderers();
    QRectF metresBox = renderers.boundingBox();

    double frameSize = 1080;;

    double scale = (double)frameSize / std::max(metresBox.width(), metresBox.height());

    QRectF centimetresBox(metresBox.left() * scale, metresBox.top() * scale, metresBox.width() * scale, metresBox.height() * scale);

    Camera3D offlineCamera;

    offlineCamera.showAll(metresBox, GLWidget::kProjectionPerspectiveAngle, static_cast<int>(centimetresBox.width()), static_cast<int>(centimetresBox.height()));

    gl.m_camera = offlineCamera.toMatrix();
    gl.m_appPalette = palette();
    gl.m_isReplay = false;
    gl.m_width = static_cast<int>(centimetresBox.width());
    gl.m_height = static_cast<int>(centimetresBox.height());

    QMatrix4x4 projection;
    projection.setToIdentity();
    projection.perspective(45.f, (float)centimetresBox.width() / (float)centimetresBox.height(), 0.1f, 1000.f);

    gl.m_projection = projection;

    // gl.m_scale = 1.0;
    gl->glViewport(0, 0, static_cast<int>(centimetresBox.width()), static_cast<int>(centimetresBox.height()));

    QOpenGLFramebufferObject fbo(static_cast<int>(centimetresBox.width()), static_cast<int>(centimetresBox.height()));
    // fbo.bind();

    QColor colour = Qt::black;
    gl->glClearColor(colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderers.propertyChanged(gui::ePropertyRenderOnlySonarData, true);
    renderers.paintGL(gl);
    renderers.propertyChanged(gui::ePropertyRenderOnlySonarData, false);


    QImage result = fbo.toImage();

    TargetTracking::TrackingImageInfo trackingImageInfo;
    trackingImageInfo.m_scale = scale;

     cv::Mat image = cv::Mat( result.height(), result.width(),
                   CV_8UC4,
                   const_cast<uchar*>(result.bits()),
                   static_cast<size_t>(result.bytesPerLine())
                   );

     if (image.empty())
     {
         return;
     }

    cvtColor(image, trackingImageInfo.m_opencvImageCm, cv::COLOR_BGR2GRAY);

    trackingImageInfo.m_origin.x = static_cast<int>(-centimetresBox.x());
    trackingImageInfo.m_origin.y = static_cast<int>(-centimetresBox.y());
    trackingImageInfo.m_time = m_renderTime;

    if (displaySettings.m_extractImagesFromLog)
    {
        cv::imwrite(displaySettings.m_extractImagesFolder + "\\" + "image" + std::to_string(static_cast<long long>(m_imageNumber++)) +
                    ".jpg", image);
    }
    else if (m_pTargetTracking != nullptr)
    {
        m_pTargetTracking->processTargetImage(trackingImageInfo);
    }
    */
}
