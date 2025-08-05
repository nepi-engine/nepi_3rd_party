#include "compassrenderer.h"
#include "shaders.h"

#include <QVector2D>
#include <QtMath>

#include "geometry.h"

//This is a 'feature' to allow the RPI+ 3 compile.
#ifndef GL_BGRA
#define GL_BGRA                           GL_BGRA_EXT
#endif


CompassRenderer::CompassRenderer()
{
    m_enabled = false;
    memset(&m_compassRec, 0, sizeof(GLF::CompassDataRecord));
}

GLuint CompassRenderer::compileProgram(const char* vertex, const char* fragment)
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

void CompassRenderer::initialiseGL(QOpenGLContext &gl)
{    
    initializeOpenGLFunctions();

#ifdef RASPI
    const char* vertex = (gl.isOpenGLES()) ? shaders::v320es::basic_texture_3d_es_vert : shaders::v110::basic_texture_3d_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v320es::basic_texture_3d_es_frag : shaders::v110::basic_texture_3d_frag;
#else
    const char* vertex = (gl.isOpenGLES()) ? shaders::v110es::basic_texture_3d_es_vert : shaders::v110::basic_texture_3d_vert;
    const char* fragment = (gl.isOpenGLES()) ? shaders::v110es::basic_texture_3d_es_frag : shaders::v110::basic_texture_3d_frag;
#endif

    m_textureProgram = compileProgram(vertex, fragment);

    m_locTexturePosition = glGetAttribLocation(m_textureProgram, "position");
    m_locTextureTexCoord = glGetAttribLocation(m_textureProgram, "texCoord");
    m_locColour = glGetUniformLocation(m_textureProgram, "colour");
    m_locTextureMVMatrix = glGetUniformLocation(m_textureProgram, "modelViewMatrix");
    m_locTextureProjMatrix = glGetUniformLocation(m_textureProgram, "projMatrix");
    m_locTextureSampler = glGetUniformLocation(m_textureProgram, "tex");

    QImage compassImg(":/icons/compass/compass.png");
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_compassTexture);
    glBindTexture(GL_TEXTURE_2D, m_compassTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, compassImg.width(), compassImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, compassImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage needleImg(":/icons/compass/compass-needle.png");
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_needleTexture);
    glBindTexture(GL_TEXTURE_2D, m_needleTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, needleImg.width(), needleImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, needleImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

#ifdef RASPI
    vertex = (gl.isOpenGLES()) ? shaders::v320es::compass_masked_es_vert : shaders::v110::compass_masked_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v320es::compass_masked_es_frag : shaders::v110::compass_masked_frag;
#else
    vertex = (gl.isOpenGLES()) ? shaders::v110es::compass_masked_es_vert : shaders::v110::compass_masked_vert;
    fragment = (gl.isOpenGLES()) ? shaders::v110es::compass_masked_es_frag : shaders::v110::compass_masked_frag;
#endif

    m_pitchRollProgram = compileProgram(vertex, fragment);

    //m_pitchRollProgram = ShaderMgr::getProgram("compass_masked", gl);

    QImage horizonImg(":/icons/compass/ArtificialHorizon_Blank.png");
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_horizonTexture);
    glBindTexture(GL_TEXTURE_2D, m_horizonTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, horizonImg.width(), horizonImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, horizonImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    QImage maskImg(":/icons/compass/HorizonMask.png");
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_maskTexture);
    glBindTexture(GL_TEXTURE_2D, m_maskTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maskImg.width(), maskImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, maskImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_glText.setFontSize(15);
    m_glText.setAllowCaching(true);
    m_glText.initialiseGL(gl);
    m_glText.setColour(Qt::white);
}

/*
 * \brief draw compass elements
 */
void CompassRenderer::paintGL(QOpenGLContext &gl)
{
    if (!m_enabled)
        return;

    paintPitchRoll(gl, m_compassRec.m_pitch, m_compassRec.m_roll);
    float heading = static_cast<float>(m_compassRec.m_heading);
    float pitch = static_cast<float>(m_compassRec.m_pitch);
    float roll = static_cast<float>(m_compassRec.m_roll);
    paintCompass(gl, heading);
    paintHPRText(gl, heading, -pitch, -roll);
}

/*
 * \brief draw a depiction of the current pitch/roll
 */
void CompassRenderer::paintPitchRoll(QOpenGLContext &gl, float pitch, float roll)
{
    static const int kElementSize = 7; // (X,Y,Z),(U0,V0,U1,V1)

    // set the shader to use and populate the variables
    glUseProgram(m_pitchRollProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    GLuint locPosition = glGetAttribLocation(m_pitchRollProgram, "position");
    glEnableVertexAttribArray(locPosition);
    GLuint locTexCoord = glGetAttribLocation(m_pitchRollProgram, "uv");
    glEnableVertexAttribArray(locTexCoord);

    GLfloat box[4 * kElementSize];
    glVertexAttribPointer(locPosition, 3, GL_FLOAT, GL_FALSE,
                              kElementSize * sizeof(GLfloat), &box[0]);
    glVertexAttribPointer(locTexCoord, 4, GL_FLOAT, GL_FALSE,
                              kElementSize * sizeof(GLfloat), &box[3]);
    glUniform4f(m_locColour, 1.f, 1.f, 1.f, 1.f);


    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(m_pitchRollProgram, "tex"), 0);
    glBindTexture(GL_TEXTURE_2D, m_horizonTexture);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(m_pitchRollProgram, "texMask"), 1);
    glBindTexture(GL_TEXTURE_2D, m_maskTexture);

    // hand transform uvs according to m_pitchDegrees/m_rollDegrees
    QVector2D uvs[4] = {
        QVector2D(0, 0),
        QVector2D(0, 1),
        QVector2D(1, 0),
        QVector2D(1, 1)
    };

    // clamp pitch as otherwise the scrolling to indicate pitch gets excessive
    if (pitch > 60.f) pitch = 60.f;
    if (pitch < -60.f) pitch = -60.f;

    const float kImageHeight = 708; // by inspection...
    const float kPixelsPerDegree = 3.6f; // 36/10, by inspection
    // pitch is an up/down shift of the background image
    float pitchShift = (pitch * kPixelsPerDegree) / kImageHeight;

    float ss = sin(qDegreesToRadians(roll));
    float cs = cos(qDegreesToRadians(roll));
    for (int n = 0; n < 4; n++)
    {
        // and rotate the uvs to represent roll
        float x = uvs[n].x() - 0.5f;
        float y = uvs[n].y() - 0.5f;
        uvs[n].setX((x*cs - y*ss) + 0.5f);
        uvs[n].setY((y*cs + x*ss) + 0.5f);

        // enact a scroll of the uvs to represent pitch
        uvs[n].setY(uvs[n].y() - pitchShift);
    }

    int width = kCompassWidth;
    int height = kCompassHeight; // give it room to scroll up/down

    // flip y due to our 3D coordinate system being right handed
    int ptIndex = 0;
    box[ptIndex++] = -width / 2.f;
    box[ptIndex++] =  height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = uvs[0].x();
    box[ptIndex++] = uvs[0].y();
    box[ptIndex++] = 0;
    box[ptIndex++] = 0;

    box[ptIndex++] = -width / 2.f;
    box[ptIndex++] = -height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = uvs[1].x();
    box[ptIndex++] = uvs[1].y();
    box[ptIndex++] = 0;
    box[ptIndex++] = 1;

    box[ptIndex++] = width / 2.f;
    box[ptIndex++] = height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = uvs[2].x();
    box[ptIndex++] = uvs[2].y();
    box[ptIndex++] = 1;
    box[ptIndex++] = 0;

    box[ptIndex++] =  width / 2.f;
    box[ptIndex++] = -height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = uvs[3].x();
    box[ptIndex++] = uvs[3].y();
    box[ptIndex++] = 1;
    box[ptIndex++] = 1;    

    // setup the projection matrix
    QMatrix4x4 projMatrix;
    float view[4];
    glGetFloatv(GL_VIEWPORT, view);
    projMatrix.ortho(view[0], view[2], view[1], view[3], -1000.f, 1000.f);
    glUniformMatrix4fv(glGetUniformLocation(m_pitchRollProgram, "projMatrix"), 1, GL_FALSE, projMatrix.constData());

    // setup the model view matrix transform (offset to TR of the view)
    QMatrix4x4 modelViewMatrix;
    modelViewMatrix.translate(view[2] - (width / 2.f) - kViewEdgeOffset, view[3] - (height / 2.f) - kViewEdgeOffset);
    glUniformMatrix4fv(glGetUniformLocation(m_pitchRollProgram, "modelViewMatrix"), 1, GL_FALSE, modelViewMatrix.constData());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // disable gl states no longer in use
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(locPosition);
    glDisableVertexAttribArray(locTexCoord);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}

/*
 * \brief draw the main compass with heading
 */
void CompassRenderer::paintCompass(QOpenGLContext &gl, float heading)
{
    // set the shader to use and populate the variables
    glUseProgram(m_textureProgram);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_locTextureSampler, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glEnableVertexAttribArray(m_locTexturePosition);
    glEnableVertexAttribArray(m_locTextureTexCoord);

    GLfloat box[4 * kTexturedElementSize];
    glVertexAttribPointer(m_locTexturePosition, 3, GL_FLOAT, GL_FALSE,
                              kTexturedElementSize * sizeof(GLfloat), &box[0]);
    glVertexAttribPointer(m_locTextureTexCoord, 2, GL_FLOAT, GL_FALSE,
                              kTexturedElementSize * sizeof(GLfloat), &box[3]);
    glUniform4f(m_locColour, 1.f, 1.f, 1.f, 1.f);

    QMatrix4x4 projMatrix;
    float view[4];
    glGetFloatv(GL_VIEWPORT, view);
    projMatrix.ortho(view[0], view[2], view[1], view[3], -1000.f, 1000.f);
    glUniformMatrix4fv(m_locTextureProjMatrix, 1, GL_FALSE, projMatrix.constData());

    int width = kCompassWidth;
    int height = kCompassHeight;

    // flip y due to our 3D coordinate system being right handed
    int ptIndex = 0;
    box[ptIndex++] = -width / 2.f;
    box[ptIndex++] =  height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = 0;
    box[ptIndex++] = 0;

    box[ptIndex++] = -width / 2.f;
    box[ptIndex++] = -height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = 0;
    box[ptIndex++] = 1;

    box[ptIndex++] = width / 2.f;
    box[ptIndex++] = height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = 1;
    box[ptIndex++] = 0;

    box[ptIndex++] =  width / 2.f;
    box[ptIndex++] = -height / 2.f;
    box[ptIndex++] = 0;
    box[ptIndex++] = 1;
    box[ptIndex++] = 1;

    QMatrix4x4 modelViewMatrix;
    // draw the main compass body
    {
        glBindTexture(GL_TEXTURE_2D, m_compassTexture);

        // setup the model view matrix transform (offset to TR of the view)
        modelViewMatrix.translate(view[2] - (width / 2.f) - kViewEdgeOffset, view[3] - (height / 2.f) - kViewEdgeOffset);
        modelViewMatrix.rotate(heading, 0.0, 0.0, 1.0);
        glUniformMatrix4fv(m_locTextureMVMatrix, 1, GL_FALSE, modelViewMatrix.constData());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // do the needle/heading draw
    {
        glBindTexture(GL_TEXTURE_2D, m_needleTexture);

        modelViewMatrix.setToIdentity();
        modelViewMatrix.translate(view[2] - (width / 2.f) - kViewEdgeOffset, view[3] - (height / 2.f) - kViewEdgeOffset);
        glUniformMatrix4fv(m_locTextureMVMatrix, 1, GL_FALSE, modelViewMatrix.constData());

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // disable gl states no longer in use
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisableVertexAttribArray(m_locTexturePosition);
    glDisableVertexAttribArray(m_locTextureTexCoord);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
}

void CompassRenderer::paintHPRText(QOpenGLContext &gl, float heading, float pitch, float roll)
{
    float view[4];
    float sw, sh, x, y, mid;
    int width, height;
    QString headingText,  pitchText, rollText;

    glGetFloatv(GL_VIEWPORT, view);

    width = kCompassWidth;
    height = kCompassHeight;

    x = view[2] - kViewEdgeOffset - width + 28; //view[2] = width of view
    y = view[3] - kViewEdgeOffset - 8;      //view[3] = height of view
    mid = view[3]/2;
    // clamp heading between 0..360
    while (heading > 360.f) heading -= 360.f;
    while (heading < 0.f) heading += 360.f;

    // B0 = unicode character code for the degree symbol
    headingText = ("H: " + QString::number(static_cast<double>(heading), 'f', 1) + QChar(0xB0)).rightJustified(10,' ');
    pitchText = ("P: " + QString::number(static_cast<double>(pitch), 'f', 1) + QChar(0xB0)).rightJustified(10,' ');
    rollText = ("R: " + QString::number(static_cast<double>(roll), 'f', 1) + QChar(0xB0)).rightJustified(10,' ');



    m_glText.getWidthAndHeight(headingText, 1.f, sw, sh);
    x -= sw / 2.f;

    m_glText.setColour(QColor(128, 128, 128));

    m_glText.draw2D(gl, headingText, QVector2D(x, y), Qt::AlignRight | Qt::AlignTop, 1.f);

    y = mid;

    m_glText.draw2D(gl, pitchText, QVector2D(x, y), Qt::AlignRight | Qt::AlignTop, 1.f);

    y = sh;

    m_glText.draw2D(gl, rollText, QVector2D(x, y), Qt::AlignRight | Qt::AlignTop, 1.f);

}

void CompassRenderer::cleanup(QOpenGLContext &gl)
{
    glDeleteTextures(1, &m_compassTexture);
    m_compassTexture = 0;

    glDeleteTextures(1, &m_needleTexture);
    m_needleTexture = 0;

    glDeleteTextures(1, &m_horizonTexture);
    m_horizonTexture = 0;

    glDeleteTextures(1, &m_maskTexture);
    m_maskTexture = 0;

    m_glText.cleanup(gl);
}

