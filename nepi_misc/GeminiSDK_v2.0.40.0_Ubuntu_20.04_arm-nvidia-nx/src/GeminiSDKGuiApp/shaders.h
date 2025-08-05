#ifndef SHADERS_H
#define SHADERS_H

#include <QtGui/QOpenGLContext>

/*
 For 320 ES:

 Shader:
 attribute -> in
 varying -> out

 Fragment:

 varying -> in
 *
 */
namespace shaders {
    namespace v320es {

        const char default_text_es_frag [] =
        "#version 320 es\n"
        "uniform sampler2D tex;\n"
        "precision mediump float;\n"
        "uniform vec4 color;\n"
        "in vec2 texpos;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    fragColor = vec4(1, 1, 1, texture2D(tex, texpos).r) * color;\n"
        "    gl_FragDepth = clamp(gl_FragCoord.z - 0.0011, 0.0, 1.0);\n"
        "}\n";

        const char default_text_es_vert [] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec4 coord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 texpos;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(coord.xy, 0.0, 1.0);\n"
        "    texpos = coord.zw;\n"
        "}\n";

        const char basic_texture_3d_es_frag [] =
        "#version 320 es\n"
        "uniform sampler2D tex;\n"
        "precision mediump float;\n"
        "uniform vec4 colour;\n"
        "in vec2 TexCoord;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord).rgba;\n"
        "    if (texCol.a == 0.0)\n"
        "        discard;\n"
        "    fragColor = texCol * colour;\n"
        "}";


        const char basic_texture_3d_es_vert [] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec3 position;\n"
        "in vec2 texCoord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "    TexCoord = texCoord;\n"
        "}";

        const char compass_masked_es_frag [] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "uniform sampler2D tex;\n"
        "uniform sampler2D texMask;\n"
        "in vec2 TexCoord0;\n"
        "in vec2 TexCoord1;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord0).rgba;\n"
        "    vec4 maskCol = texture2D(texMask, TexCoord1).rgba;\n"
        "    fragColor = vec4(texCol.rgb, maskCol.a);\n"
        "}";

        const char compass_masked_es_vert [] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec3 position;\n"
        "in vec4 uv;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 TexCoord0;\n"
        "out vec2 TexCoord1;\n"
        "void main()\n"
        "{\n"
        "    TexCoord0 = uv.xy;\n"
        "    TexCoord1 = uv.zw;\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "}";

        // vertex shader for OpenGL ES
        const char sonar_es_vert [] =
        "#version 320 es\n"
        "in vec2 position;\n"
        "in vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "out vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "    textureCoord = texCoord;\n"
        "}\n"
        ;

        // fragment shader for OpenGL ES
        const char sonar_es_frag [] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "in vec2 textureCoord;\n"
        "uniform sampler2D sonarTexture;\n"
        "uniform sampler2D paletteTexture;\n"
        "out vec4 fragColor;\n"
        "out vec4 index;\n"
        "void main()\n"
        "{\n"
        "    index = texture2D(sonarTexture, textureCoord);\n"
        "    fragColor = vec4(texture2D(paletteTexture, index.xy).rgb, 1.0);\n"
        "}\n"
        ;

        // vertex shader for OpenGL ES
        const char text_es_vert[] =
        "#version 320 es\n"
        "in vec2 position;\n"
        "in vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "out vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    textureCoord = texCoord;\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "}\n"
        ;

        // fragment shader for OpenGL ES
        const char text_es_frag[] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "out vec2 textureCoord;\n"
        "uniform sampler2D texture;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = texture2D(texture, textureCoord);\n"
        "}\n"
        ;

        // standard default vertex shader for OpenGL ES
        const char default_es_vert[] =
        "#version 320 es\n"
        "in vec3 position;\n"
        "uniform mat4 matrix;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n"
        ;

        // standard default fragment shader for OpenGL ES
        const char default_es_frag[] =
        "#version 320 es\n"
        "precision mediump float;\n"
        "uniform vec4 color;\n"
        "out vec4 fragColor;\n"
        "void main()\n"
        "{\n"
        "    fragColor = color;\n"
        "}\n"
        ;
    }

    namespace v110es {
        const char default_text_es_frag [] =
        "#version 110 es\n"
        "uniform sampler2D tex;\n"
        "precision mediump float;\n"
        "uniform vec4 color;\n"
        "in vec2 texpos;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    fragColor = vec4(1, 1, 1, texture2D(tex, texpos).r) * color;\n"
        "    gl_FragDepth = clamp(gl_FragCoord.z - 0.0011, 0.0, 1.0);\n"
        "}\n";

        const char default_text_es_vert [] =
        "#version 110 es\n"
        "precision mediump float;\n"
        "in vec4 coord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 texpos;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(coord.xy, 0.0, 1.0);\n"
        "    texpos = coord.zw;\n"
        "}\n";

        const char basic_texture_3d_es_frag [] =
        "#version 110 es\n"
        "uniform sampler2D tex;\n"
        "precision mediump float;\n"
        "uniform vec4 colour;\n"
        "in vec2 TexCoord;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord).rgba;\n"
        "    if (texCol.a == 0.0)\n"
        "        discard;\n"
        "    fragColor = texCol * colour;\n"
        "}";

        const char basic_texture_3d_es_vert [] =
        "#version 110 es\n"
        "precision mediump float;\n"
        "in vec3 position;\n"
        "in vec2 texCoord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "    TexCoord = texCoord;\n"
        "}";

        const char compass_masked_es_frag [] =
        "#version 110 es\n"
        "precision mediump float;\n"
        "uniform sampler2D tex;\n"
        "uniform sampler2D texMask;\n"
        "in vec2 TexCoord0;\n"
        "in vec2 TexCoord1;\n"
        "out vec4 fragColor;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord0).rgba;\n"
        "    vec4 maskCol = texture2D(texMask, TexCoord1).rgba;\n"
        "    fragColor = vec4(texCol.rgb, maskCol.a);\n"
        "}";

        const char compass_masked_es_vert [] =
        "#version 110 es\n"
        "precision mediump float;\n"
        "in vec3 position;\n"
        "in vec4 uv;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "out vec2 TexCoord0;\n"
        "out vec2 TexCoord1;\n"
        "void main()\n"
        "{\n"
        "    TexCoord0 = uv.xy;\n"
        "    TexCoord1 = uv.zw;\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "}";

    }
    namespace v100es {

        // vertex shader for OpenGL ES
        const char sonar_es_vert [] =
        "#version 100 es\n"
        "attribute vec2 position;\n"
        "attribute vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "varying vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "    textureCoord = texCoord;\n"
        "}\n"
        ;

        // fragment shader for OpenGL ES
        const char sonar_es_frag [] =
        "#version 100 es\n"
        "precision mediump float;\n"
        "varying vec2 textureCoord;         // sonar texture coordinate\n"
        "uniform sampler2D sonarTexture;     // sonar image\n"
        "uniform sampler2D paletteTexture;   // 256 x 1 pixels\n"
        "void main()\n"
        "{\n"
        "    // get palette index from sonarTexture (intensity)\n"
        "    vec4 index = texture2D(sonarTexture, textureCoord);\n"
        "    // get colour from palette\n"
        "    gl_FragColor = vec4(texture2D(paletteTexture, index.xy).rgb, 1.0);\n"
        "}\n"
        ;

        // vertex shader for OpenGL ES
        const char text_es_vert[] =
        "#version 100 es\n"
        "attribute vec2 position;\n"
        "attribute vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "varying vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "    textureCoord = texCoord;\n"
        "}\n"
        ;

        // fragment shader for OpenGL ES
        const char text_es_frag[] =
        "#version 100 es\n"
        "precision mediump float;\n"
        "varying vec2 textureCoord;\n"
        "uniform sampler2D texture;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, textureCoord);\n"
        "}\n"
        ;

        // standard default vertex shader for OpenGL ES
        const char default_es_vert[] =
        "#version 100 es\n"
        "attribute vec3 position;\n"
        "uniform mat4 matrix;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n"
        ;

        // standard default fragment shader for OpenGL ES
        const char default_es_frag[] =
        "#version 100 es\n"
        "precision mediump float;\n"
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n"
        ;
    }

    namespace v110 {
        const char default_text_frag [] =
        "#version 110\n"
        "uniform sampler2D tex;\n"
        "uniform vec4 color;\n"
        "varying vec2 texpos;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = vec4(1, 1, 1, texture2D(tex, texpos).r) * color;\n"
        "    gl_FragDepth = clamp(gl_FragCoord.z - 0.0011, 0.0, 1.0);\n"
        "}\n";

        const char default_text_vert [] =
        "#version 110\n"
        "attribute vec4 coord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "varying vec2 texpos;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(coord.xy, 0.0, 1.0);\n"
        "    texpos = coord.zw;\n"
        "}\n";

        const char basic_texture_3d_frag [] =
        "#version 110\n"
        "uniform sampler2D tex;\n"
        "uniform vec4 colour;\n"
        "varying vec2 TexCoord;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord).rgba;\n"
        "    if (texCol.a == 0.0)\n"
        "        discard;\n"
        "    gl_FragColor = texCol * colour;\n"
        "}";


        const char basic_texture_3d_vert [] =
        "#version 110\n"
        "attribute vec3 position;\n"
        "attribute vec2 texCoord;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "varying vec2 TexCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "    TexCoord = texCoord;\n"
        "}";

        const char compass_masked_frag [] =
        "#version 110\n"
        "uniform sampler2D tex;\n"
        "uniform sampler2D texMask;\n"
        "varying vec2 TexCoord0;\n"
        "varying vec2 TexCoord1;\n"
        "void main(void)\n"
        "{\n"
        "    vec4 texCol = texture2D(tex, TexCoord0).rgba;\n"
        "    vec4 maskCol = texture2D(texMask, TexCoord1).rgba;\n"
        "    gl_FragColor = vec4(texCol.rgb, maskCol.a);\n"
        "}";

        const char compass_masked_vert [] =
        "#version 110\n"
        "attribute vec3 position;\n"
        "attribute vec4 uv;\n"
        "uniform mat4 projMatrix;\n"
        "uniform mat4 modelViewMatrix;\n"
        "varying vec2 TexCoord0;\n"
        "varying vec2 TexCoord1;\n"
        "void main()\n"
        "{\n"
        "    TexCoord0 = uv.xy;\n"
        "    TexCoord1 = uv.zw;\n"
        "    gl_Position = projMatrix * modelViewMatrix * vec4(position, 1.0);\n"
        "}";

        // vertex shader for OpenGL GLSL
        const char sonar_vert [] =
        "#version 110\n"
        "attribute vec2 position;\n"
        "attribute vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "varying vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "    textureCoord = texCoord;\n"
        "}\n"
        ;

        // fragment shader for OpenGL GLSL
        const char sonar_frag [] =
        "#version 110\n"
        "varying vec2 textureCoord;         // sonar texture coordinate\n"
        "uniform sampler2D sonarTexture;     // sonar image\n"
        "uniform sampler2D paletteTexture;   // 256 x 1 pixels\n"
        "void main()\n"
        "{\n"
        "    // get palette index from sonarTexture (intensity)\n"
        "    vec4 index = texture2D(sonarTexture, textureCoord);\n"
        "    // get colour from palette\n"
        "    gl_FragColor = vec4(texture2D(paletteTexture, index.xy).rgb, 1.0);\n"
        "}\n"
        ;

        // vertex shader for OpenGL GLSL
        const char text_vert[] =
        "#version 110\n"
        "attribute vec2 position;\n"
        "attribute vec2 texCoord;\n"
        "uniform mat4 matrix;\n"
        "varying vec2 textureCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 0.0, 1.0);\n"
        "    textureCoord = texCoord;\n"
        "}\n"
        ;

        // fragment shader for OpenGL GLSL
        const char text_frag[] =
        "#version 110\n"
        "varying vec2 textureCoord;\n"
        "uniform sampler2D texture;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, textureCoord);\n"
        "}\n"
        ;

        // standard default vertex shader for OpenGL GLSL
        const char default_vert[] =
        "#version 110\n"
        "attribute vec3 position;\n"
        "uniform mat4 matrix;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = matrix * vec4(position, 1.0);\n"
        "}\n"
        ;

        // standard default fragment shader for OpenGL GLSL
        const char default_frag[] =
        "#version 110\n"
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n"
        ;
    }

    namespace v100 {


    }
}   // namespace shaders


#endif // SHADERS_H
