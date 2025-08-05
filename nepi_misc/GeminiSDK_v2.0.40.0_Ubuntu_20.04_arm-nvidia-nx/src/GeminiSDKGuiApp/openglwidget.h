#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QtWidgets/QOpenGLWidget>
#include "GenesisSerializer/GlfApi.h"
#ifdef _ECD_LOGGER_
#include "ECDLogDataTypes/ecdlogtargetimage.h"
#endif
#include "geminirenderer.h"

#if ( _MSC_VER > 1900 ) // supported in VS2017 or above
#include <mutex>          // std::mutex
#endif

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
#if ( _MSC_VER > 1900 ) // supported in VS2017 or above
    ~OpenGLWidget() override;
#else
     virtual ~OpenGLWidget();
#endif
    // accept mouse wheel events for zooming
    void enableMouseWheelZoom(bool enable) { m_enableWheelZoom = enable; }
    void setRange(double range);
    double getRange() const;
    void enableText(bool enable) { m_drawText = enable; }
    void enableGrid(bool enable) { m_drawGrid = enable; }
    void clearImage();

public slots:
    void onGlfGeminiImage(const GLF::GLogTargetImage& image);
#ifdef _ECD_LOGGER_
    void onEcdGeminiImage(const CTgtImg& image);
#endif
    void onSetZoom(float zoom); // Set the zoom value, < 1.0 zooms out, > 1.0 zooms in.   
signals:
    void infoMessage(const QString& message, int timeout = 3000);
    void sizeChanged(int width, int height);
    void rangeChanged(double range);
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

private:
    void cleanup();                 // free allocated OpenGL resources

private:
    QMatrix4x4 m_projection;        // projection matrix
    float m_zoom;                   // zoom factor used as inverse square for zoom distance.
    bool m_enableWheelZoom;         // allow zooming with mouse wheel.

    bool m_drawGrid;                // draw the sonar grid overlay
    bool m_drawText;                // draw the range labels
    bool m_glfLogger;
#if ( _MSC_VER > 1900 ) // supported in VS2017 or above
    std::mutex m_renderMutex;           // mutex for critical section
    void LockRenderer() { m_renderMutex.lock(); }
    void UnlockRenderer() { m_renderMutex.unlock(); }
#else
    void LockRenderer() { }
    void UnlockRenderer() { }
#endif
    bool m_cleanupRender;
    // associate node IDs with a renderer
    std::map<int, GeminiRenderer*> m_renderers;
};

#endif
