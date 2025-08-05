#include "openglgrid.h"

#include <QtCore/QDebug>
#include <QtGui/QMatrix4x4>


#ifdef USE_GLES
#include <GLES3/gl3.h> 
#else
#include <GL/gl.h> 
#endif

#include <math.h>


OpenGLGrid::OpenGLGrid()
:   m_program(0)
,   m_coordAttribute(0)
,   m_colorUniform(-1)
,   m_matrixUniform(-1)
,   m_range(0.0)
,   m_startAngle(0.0)
,   m_endAngle(0.0)
{

}

void OpenGLGrid::initialiseGL(GLuint program)
{
    initializeOpenGLFunctions();

    m_program = program;

    if (0 != m_program)
    {
        m_coordAttribute = glGetAttribLocation(m_program, "position");
        if (m_coordAttribute == -1)
        {
            qDebug() << "OpenGL GLSL could not bind attribute: position";
        }

        m_colorUniform = glGetUniformLocation(m_program, "color");
        if (m_colorUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: color";
        }

        m_matrixUniform = glGetUniformLocation(m_program, "matrix");
        if (m_matrixUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: matrix";
        }
    }
}

void OpenGLGrid::paintGL(const QMatrix4x4& matrix, double range,
                         std::vector<double>& ranges,
                         double startAngle, double endAngle)
{
    if (0 == m_program) return;

    setGeometry(range, ranges, startAngle, endAngle);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
#ifdef WIN32
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_MULTISAMPLE);
#endif
    glLineWidth(1.5f);

    glUseProgram(m_program);

    glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, matrix.constData());
    glUniform4f(m_colorUniform, 0.5f, 0.5f, 0.5f, 1.0f);

    glEnableVertexAttribArray(m_coordAttribute);

    for(LineStrips::iterator it = m_lines.begin(); it != m_lines.end(); ++it)
    {
        std::vector<float>& vertices = *it;
        if (vertices.empty()) continue;

        glVertexAttribPointer(m_coordAttribute, sVertexSize, GL_FLOAT, GL_FALSE,
                              sVertexSize * sizeof(GLfloat), &vertices[0]);

        glDrawArrays(GL_LINE_STRIP, 0, vertices.size() / sVertexSize);
    }

    glDisableVertexAttribArray(m_coordAttribute);

    glUseProgram(0);    // release program

    glDisable(GL_BLEND);
}

void OpenGLGrid::cleanup()
{
    glDeleteShader(m_program);
    m_program = 0;
}

void OpenGLGrid::setGeometry(double range, std::vector<double>& ranges, double startAngle, double endAngle)
{
    if (m_range == range && startAngle == m_startAngle && endAngle == m_endAngle)
    {
        return;     // skip if not changed
    }

    m_range = range;
    m_startAngle = startAngle;
    m_endAngle = endAngle;

    double start = (startAngle + 90.0) * M_PI / 180.0;
    double end = (endAngle + 90.0) * M_PI / 180.0;
    double step = (start - end) / 360.0;
    unsigned int segments = 361;

    m_lines.resize(ranges.size() + 2);
    LineStrips::iterator it = m_lines.begin();

    // create the grid arcs
    for (unsigned int i = 0; i < ranges.size(); ++i)
    {
        std::vector<float>& vertices = *it;
        vertices.resize(sVertexSize * segments);

        double r = ranges[i] / range;
        for (unsigned int v = 0; v < segments; ++v)
        {
            double ang =  start - (step * (v));
            vertices[v * sVertexSize]   = cos(ang) * r;
            vertices[v * sVertexSize+1] = sin(ang) * r;
        }
        ++it;
    }

    // create the left/right border lines
    std::vector<float>& v = *it;
    v.resize(sVertexSize * 3);
    Q_ASSERT(2 == sVertexSize);
    v[0] = cos(start);
    v[1] = sin(start);
    v[2] = 0.0;
    v[3] = 0.0;
    v[4] = cos(end);
    v[5] = sin(end);

    // create centre line
    std::vector<float>& v1 = *(++it);
    v1.resize(sVertexSize * 2);
    v1[0] = 0.0;
    v1[1] = 1.0;
    v1[2] = 0.0;
    v1[3] = 0.0;
}


