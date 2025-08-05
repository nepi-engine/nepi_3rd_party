#ifndef GEMINIRENDERER_H
#define GEMINIRENDERER_H

#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QMatrix4x4>
#include "GenesisSerializer/GlfApi.h"

#ifdef _ECD_LOGGER_
#include "ECDLogDataTypes/ecdlogtargetimage.h"
#endif

#include "openglgrid.h"
#include "opengltext.h"

class GeminiRenderer : protected QOpenGLFunctions
{    
public:
    GeminiRenderer();

    void initialiseGL(QOpenGLContext& gl);
    void cleanup(QOpenGLContext& gl);
    void paintGL(QOpenGLContext& gl, float zoom, int width, int height, bool glfLogger, bool drawText, bool drawGrid);
    void resizeGL(int width, int height);

    void setVertices(const GLF::GLogTargetImage& image); // define vertices for sonar data mesh
    bool hasRangeChanged(const GLF::GLogTargetImage& image, double *new_range);   // sets the internal range and returns the value.  For Broadcast Client Range updates.

#ifdef _ECD_LOGGER_
    void setVertices(const CTgtImg& image);
    bool hasRangeChanged(const CTgtImg& image, double *new_range);
#endif
private:
    GLuint compileProgram(const char* vertex, const char* fragment);    // compile shaders into a program

    void paintSonarImage(const float* mvp, bool glfLogger);

    bool saveXYToBMP(int width, int height, std::string& filename, bool glfLogger);
    bool SaveBMP(const int width, const int height, const unsigned char* pixels, const char* path);

    void initPalette();
    void getRangeLabels(double range, std::vector<double>& ranges) const;

    double CalculateRangeInMeters(double, bool);

    void drawOfflineImage(/*QOpenGLContext& gl, */bool glfLogger);

private:
    GLuint m_program;               // shader program
    GLuint m_paletteID;             // texture ID for palette
    GLuint m_textureID;             // texture ID for sonar image
    GLint m_paletteUniform;         // shader uniform for palette
    GLint m_textureUniform;         // shader uniform for sonar texture
    GLint m_matrixUniform;          // shader uniform for matrix (projection * model)

    std::vector<float> m_vertices;  // vertices for sonar image mesh
    unsigned int m_bearings;        // number of bearing of current image
#ifdef _ECD_LOGGER_
    CTgtImg              m_ecdImage;// current sonar image to draw
#endif
    GLF::GLogTargetImage m_image;
    float m_aspectRatio;            // aspect ratio of the sonar data.

    OpenGLGrid m_grid;              // overlay grid

    OpenGLText m_text;              // range labels drawing functionality
    std::vector<double> m_ranges;   // range label values
    double m_range;                 // sonar's range

    static const unsigned int s_elementSize = 4;    // XYST (vertex coordinate + texture coordinate)
};

#endif // GEMINIRENDERER_H
