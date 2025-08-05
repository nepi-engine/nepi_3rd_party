#ifndef OPENGLCONTEXT_H
#define OPENGLCONTEXT_H

#include <QOpenGLFunctions>
#include <QMatrix4x4>
#include <QPalette>
//#include <palettes/palettes.h>

class QOpenGLContext;


class GLContext
{
public:
    GLContext(QOpenGLContext* context);

    QOpenGLFunctions* operator->();

    QOpenGLContext* m_context;
    QMatrix4x4 m_projection;
    QMatrix4x4 m_camera;
    //Palettes m_palettes;
    QPalette m_appPalette;
    int m_width;
    int m_height;
    float m_scale;
    bool m_isReplay;
};

#endif // OPENGLCONTEXT_H
