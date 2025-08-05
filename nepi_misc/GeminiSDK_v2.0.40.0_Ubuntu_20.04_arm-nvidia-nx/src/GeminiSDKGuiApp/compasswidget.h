#ifndef COMPASSWIDGET_H
#define COMPASSWIDGET_H

#include <QtWidgets/QOpenGLWidget>

#ifdef _ECD_LOGGER_
#include "ECDLogDataTypes/ecdlogtargetimage.h"
#endif
#include "compassrenderer.h"

#if ( _MSC_VER > 1900 ) // supported in VS2017 or above
#include <mutex>          // std::mutex
#endif

class CompassWidget : public QOpenGLWidget
{
    Q_OBJECT


public:
    CompassWidget(QWidget *parent = nullptr);
#if ( _MSC_VER > 1900 ) // supported in VS2017 or above
    ~CompassWidget() override;
#else
     virtual ~CompassWidget();
#endif
    void clearImage();
public slots:
    void onCompassData(GLF::CompassDataRecord* cdr);
#ifdef _ECD_LOGGER_
    void onEcdCompassData(GLF::CompassDataRecord* cdr);
#endif
    void onSetZoom(float zoom); // Set the zoom value, < 1.0 zooms out, > 1.0 zooms in.

signals:
    void infoMessage(const QString& message, int timeout = 3000);
    void sizeChanged(int width, int height);


protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* event) Q_DECL_OVERRIDE;

private:

    void cleanup();                 // free allocated OpenGL resources
    bool m_glfLogger;
    float m_zoom;                   // zoom factor used as inverse square for zoom distance.
    bool m_enableWheelZoom;         // allow zooming with mouse wheel.
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
    std::map<int, CompassRenderer*> m_renderers;
};

#endif // COMPASSWIDGET_H
