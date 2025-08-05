#ifndef GLTEXT_H
#define GLTEXT_H

#include <QtGui/QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QColor>
#include <QPoint>
#include <map>


#include "ft2build.h"
#include FT_FREETYPE_H

struct GlyphData
{
    float u[2];             // u, v settings required to draw
    float v[2];
    QPoint loc;             // x,y location on the given texture
    float width;            // current width
    float height;           // current height
    float bitmap_left;      // stored FT glyph information
    float bitmap_top;       // stored FT glyph information
};

class GLText : protected QOpenGLFunctions
{    
    static FT_Library ms_ft;
    static int ms_instanceCount;

public:
    GLText();
    ~GLText();

    void initialiseGL(QOpenGLContext& gl);
    void initialiseGL(QOpenGLContext& gl, const QString& fontPath);
    void cleanup(QOpenGLContext& gl);
    void draw2D(QOpenGLContext& gl, const QString& text, const QVector2D& pos, Qt::Alignment alignment, float scale);
    void setFontSize(unsigned int fontsize);
    void setColour(const QColor& colour);
    void setAllowCaching(bool b); // only respected in 2D drawing for now
    void getWidthAndHeight(const QString& text, float scale, GLfloat &width, GLfloat &height);

private:
    GLuint compileProgram(const char* vertex, const char* fragment);
    void convertMonochromeData(unsigned char* pData, FT_GlyphSlot glyph);
    void createGlyphCacheTexture(QOpenGLContext& gl);
    void generateGlyphSlots();
    void getGlyphData(QOpenGLContext& gl, FT_ULong chr, GlyphData& outGlyph, GLuint& currentTexture);
    bool cacheGlyph(QOpenGLContext& gl, FT_ULong chr, GlyphData& outGlyph, GLuint& currentTexture);
    void setGLTexture(QOpenGLContext& gl, GLuint textureID);

    // if caching is on it will attempt to load all glyphs onto the texture
    unsigned int preloadGlyphs(QOpenGLContext& gl, const QString& text);

    QString resolveFontPath(const QString& fontPath);

private:
#ifndef NDEBUG
    bool m_cleanupCalled;
#endif
    GLuint m_program;
    GLuint m_textureID;
    GLint  m_textureUniform;
    GLint  m_coordAttribute;
    GLint  m_colourUniform;
    GLint  m_mvMatrixUniform;
    GLint  m_projMatrixUniform;
    GLuint m_vbo;

    FT_Face m_face;

    unsigned int m_fontsize;
    QColor m_colour;
    float m_offset;

    bool m_monochrome;

    bool m_allowCaching;
    GLuint m_cachedGlyphTexture;
    std::map<FT_ULong, GlyphData> m_cachedGlyphMap;
    std::vector<GlyphData> m_freeGlyphSlots;
    std::vector<GlyphData> m_usedGlyphSlots;
    std::map<ushort, FT_Vector> m_cachedGlyphAdvance; // don't keep loading up glyphs for this
    void LoadGlyph();
};

#endif // GLTEXT_H
