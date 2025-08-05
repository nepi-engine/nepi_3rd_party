#include "opengltext.h"

#include <QtCore/QDebug>
#include <QtWidgets/QLabel>
#include <QtGui/QMatrix4x4>


OpenGLText::OpenGLText()
:   m_program(0)
,   m_coordAttribute(0)
,   m_texCoordAttribute(0)
,   m_textureUniform(-1)
,   m_matrixUniform(-1)
,   m_range(0.0)
,   m_screen(0)
{

}

void OpenGLText::initialiseGL(GLuint program)
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

        m_texCoordAttribute = glGetAttribLocation(m_program, "texCoord");
        if (m_texCoordAttribute == -1)
        {
            qDebug() << "OpenGL GLSL could not bind attribute: texCoord";
        }

        m_textureUniform = glGetUniformLocation(m_program, "texture");
        if (m_textureUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: texture";
        }

        m_matrixUniform = glGetUniformLocation(m_program, "matrix");
        if (m_matrixUniform == -1)
        {
            qDebug() << "OpenGL GLSL could not bind uniform: matrix";
        }
    }
}

void OpenGLText::paintGL(const QMatrix4x4& matrix, int size, double range, std::vector<double>& ranges,
                         double startAngle, double endAngle, double zoom)
{
    m_screen = size;

    if (0 == m_program) return;
    if (0 == m_screen) return;

    setRange(range, ranges);

    if (0 == m_labelCount) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(m_program);

    glEnableVertexAttribArray(m_coordAttribute);
    glEnableVertexAttribArray(m_texCoordAttribute);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(m_textureUniform, 0);

    std::vector<float> vertices;
    vertices.resize(16, 0.0);

    for (unsigned int i = 0 ; i < m_labelCount; i++)
    {
        const GLTextData& label = m_labels.at(i);

        const QRectF r = label.m_rect;
        if (r.isEmpty()) continue;

        float scale = ((0 == m_screen) ? 0.02 : (r.height() / m_screen) * 0.15) * (1.0 / zoom);

        glBindTexture(GL_TEXTURE_2D, label.m_textureID);

        // bottom left
        vertices[0] = r.x();
        vertices[1] = r.y();
        vertices[2] = 0.0f;
        vertices[3] = 1.0f;
        // top left
        vertices[4] = r.x();
        vertices[5] = r.y() + (r.height() * scale);
        vertices[6] = 0.0f;
        vertices[7] = 0.0f;
        // bottom right
        vertices[8] = r.x() + (r.width() * scale);
        vertices[9] = r.y();
        vertices[10] = 1.0f;
        vertices[11] = 1.0f;
        // top right
        vertices[12] = r.x() + (r.width() * scale);
        vertices[13] = r.y() + (r.height() * scale);
        vertices[14] = 1.0f;
        vertices[15] = 0.0f;


        glVertexAttribPointer(m_coordAttribute, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(GLfloat), &vertices[0]);
        glVertexAttribPointer(m_texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                              4 * sizeof(GLfloat), &vertices[2]);

        bool isLast = (i == m_labelCount - 1);

        // draw start angle labels
        QMatrix4x4 m = matrix;
        m.rotate(startAngle, 0.0, 0.0, 1.0);
        const float *test = m.constData();
        if (isLast)
        {
            m.translate(r.width() * scale * 0.02f, 0.0f, 0.0f);
        }
        else
        {
            m.translate(-r.width() * scale * 1.02f, -r.height() * scale * 0.5f, 0.0f);
        }
        const float *test2 = m.constData();
        glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, m.constData());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // draw end angle labels
        m = matrix;
        m.rotate(endAngle, 0.0, 0.0, 1.0);
        const float *test3 = m.constData();
        if (isLast)
        {
            m.translate(-r.width() * scale * 1.02f, 0.0f, 0.0f);
        }
        else
        {
            m.translate(r.width() * scale * 0.02f, -r.height() * scale * 0.5f, 0.0f);
        }
        const float *test4 = m.constData();
        glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, m.constData());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindTexture(GL_TEXTURE_2D, 0);    // unbind

    glDisableVertexAttribArray(m_coordAttribute);
    glDisableVertexAttribArray(m_texCoordAttribute);

    glUseProgram(0);    // release program

    glDisable(GL_BLEND);
}

void OpenGLText::cleanup()
{
    for (std::vector<GLTextData>::iterator it = m_labels.begin(); it != m_labels.end(); ++it)
    {
        GLTextData& label = *it;
        glDeleteTextures(1, &label.m_textureID);
        label.m_textureID = 0;
    }

    glDeleteShader(m_program);
    m_program = 0;
}

void OpenGLText::setRange(double range, std::vector<double>& ranges)
{
    if (m_range != range)
    {
        m_range = range;

        m_labelCount = ranges.size();
        if (m_labels.size() < m_labelCount)
        {
            m_labels.resize(m_labelCount);
        }

        // use Qt QLabel to generate text images to render with OpenGL as textures
        QLabel label;
        label.setStyleSheet("QLabel{background:transparent}");
        QFont f = label.font();
        f.setPointSize((f.pointSize()) * 2);
        f.setBold(true);
        label.setFont(f);

        unsigned int i = 0;
        foreach(double r, ranges)
        {
            GLTextData& data = m_labels[i++];
            data.m_range = r;

            if (r == range)
            {
                label.setText(QString ("%1m").arg(r, 0, 'f', 1));
            }
            else
            {
                label.setText(QString ("%1").arg(r, 0, 'f', 2));
            }

            label.adjustSize();

            QImage image (label.grab().toImage());
            image = image.convertToFormat(QImage::Format_RGBA8888);
            if (0 == image.width() || 0 == image.height())
            {
                data.m_rect = QRectF();
                continue;
            }

            if (0 == data.m_textureID)
            {
                // initialise the texture
                glGenTextures(1, &data.m_textureID);
                glBindTexture(GL_TEXTURE_2D, data.m_textureID);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, data.m_textureID);
            }

            // update texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
                         0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

            data.m_rect = QRectF(0.0f,
                                 data.m_range / m_range,
                                 label.width() / 2.0f,
                                 label.height() / 2.0f);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

