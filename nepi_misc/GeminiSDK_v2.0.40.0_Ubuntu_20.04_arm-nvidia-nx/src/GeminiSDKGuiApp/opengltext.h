#ifndef OPENGLTEXT_H
#define OPENGLTEXT_H

#include <QtCore/QList>
#include <QtGui/QImage>
#include <QtGui/QOpenGLFunctions>


class OpenGLText : protected QOpenGLFunctions
{
public:
    OpenGLText();

    void initialiseGL(GLuint program);
    void paintGL(const QMatrix4x4& matrix, int size, double range, std::vector<double>& ranges,
        double startAngle, double endAngle, double zoom);
    void cleanup();    

private:
    void setRange(double range, std::vector<double>& ranges);

private:
    GLuint m_program;               // shader program
    GLint m_coordAttribute;         // vertex coordinate data
    GLint m_texCoordAttribute;      // texture coordinate data
    GLint m_textureUniform;         // shader uniform for sonar texture
    GLint m_matrixUniform;          // shader uniform for matrix (projection * model)

    double m_range;                 // current range

    typedef struct GLTextData
    {
        GLTextData() : m_range (0.0), m_textureID (0) {}

        double m_range;                 // range value for label
        QRectF m_rect;                  // label geometry
        GLuint m_textureID;             // texture ID for label

    } GLTextData;

    size_t                  m_labelCount;         // number of labels to draw
    std::vector<GLTextData> m_labels;   // labels to draw

    int m_screen;
};

#endif // OPENGLTEXT_H
