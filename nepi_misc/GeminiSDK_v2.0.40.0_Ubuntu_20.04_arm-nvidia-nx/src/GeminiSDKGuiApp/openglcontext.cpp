#include "openglcontext.h"

#include <QOpenGLContext>


GLContext::GLContext(QOpenGLContext* context)
:   m_context(context)
{

}

QOpenGLFunctions* GLContext::operator->()
{
    return m_context->functions();
}
