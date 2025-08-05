#ifndef OPENGLGRID_H
#define OPENGLGRID_H

#include <QtGui/QOpenGLFunctions>


class OpenGLGrid : protected QOpenGLFunctions
{
public:
    OpenGLGrid();

    void initialiseGL(GLuint program);
    void paintGL(const QMatrix4x4& matrix, double range, std::vector<double>& ranges,
        double startAngle, double endAngle);
    void cleanup();

private:
    void setGeometry(double range, std::vector<double>& ranges, double startAngle, double endAngle);

private:
    GLuint m_program;               // shader program
    GLint m_coordAttribute;         // vertex coordinate data
    GLint m_colorUniform;           // line colour
    GLint m_matrixUniform;          // shader uniform for matrix (projection * model)

    typedef std::vector<std::vector<float>> LineStrips;
    LineStrips m_lines;  // line vertices

    double m_range;                 // sonar range
    double m_startAngle;            // sonar start angle
    double m_endAngle;              // sonar end angle

    static const unsigned int sVertexSize = 2; // XYZ
};

#endif // OPENGLGRID_H
