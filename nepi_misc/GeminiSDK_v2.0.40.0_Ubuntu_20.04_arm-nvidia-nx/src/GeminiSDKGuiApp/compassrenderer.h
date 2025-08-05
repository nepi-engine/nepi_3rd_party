#ifndef COMPASSRENDERER_H
#define COMPASSRENDERER_H

#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLContext>

#include "GenesisSerializer/CompassRecord.h"
#include "gltext.h"

class CompassRenderer : protected QOpenGLFunctions
{
public:
    CompassRenderer();

    void initialiseGL(QOpenGLContext& gl);
    void cleanup(QOpenGLContext& gl);
    void paintGL(QOpenGLContext& gl);
    void resizeGL(int width, int height);

    void setEnabled(bool enabled) { m_enabled = enabled; }
    void setCompassRec(const GLF::CompassDataRecord& compassRec) { m_compassRec = compassRec; }
    void setHeading(float headingDegs) { m_compassRec.m_heading = headingDegs; }

private:
    void paintPitchRoll(QOpenGLContext& gl, float pitch, float roll);
    void paintCompass(QOpenGLContext& gl, float heading);
    void paintHPRText(QOpenGLContext &gl, float heading, float pitch, float roll);
    GLuint compileProgram(const char* vertex, const char* fragment);

    bool m_enabled;
    GLF::CompassDataRecord m_compassRec;

    GLuint m_textureProgram;
    GLint m_locTexturePosition;
    GLint m_locTextureTexCoord;
    GLint m_locColour;
    GLint m_locTextureMVMatrix;
    GLint m_locTextureProjMatrix;
    GLint m_locTextureSampler;

    GLuint m_pitchRollProgram;

    GLuint m_compassTexture;
    GLuint m_needleTexture;
    GLuint m_horizonTexture;
    GLuint m_maskTexture;
    GLText m_glText;
};



#endif // COMPASSRENDERER_H
