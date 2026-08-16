// glad de mentira, SOLO para comprobar tipos sin GPU ni contexto GL.
//
// POR QUE EXISTE. examples/level_editor.cpp es el fichero mas grande del
// repo y el unico que no se podia comprobar de ninguna forma: glad se
// descarga al configurar CMake, asi que en un entorno sin la descarga no
// habia manera ni de pasarle -fsyntax-only. El resultado practico fue que
// la primera pantalla de proyectos se entrego siendo un printf a stdout,
// porque lo headless si se podia probar y lo de dentro del editor no.
//
// Esto NO dibuja nada. Declara los simbolos GL que usa el editor para que
// el compilador pueda comprobar el resto: sintaxis, tipos, firmas, uso de
// las APIs del propio motor. Los errores de GL de verdad (shaders,
// estados) siguen necesitando ejecutarlo.
//
// Se usa con tools/comprobar_editor.sh.

#pragma once
#define __gl_h_
#define GLFW_INCLUDE_NONE
#include <cstddef>
typedef unsigned int GLenum; typedef float GLfloat; typedef int GLint;
typedef unsigned int GLuint; typedef unsigned char GLboolean; typedef int GLsizei;
typedef char GLchar; typedef unsigned int GLbitfield; typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr; typedef unsigned char GLubyte;
#define STUBGL(n) 0x1
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_DEPTH_BUFFER_BIT 0x100
#define GL_TEXTURE_2D 0xDE1
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RGBA8 0x8058
#define GL_UNSIGNED_BYTE 0x1401
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_PACK_ALIGNMENT 0x0D05
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_BLEND 0x0BE2
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DEPTH_TEST 0x0B71
inline void glClearColor(float,float,float,float){}
inline void glClear(GLbitfield){}
inline void glFinish(){}
inline void glEnable(GLenum){}
inline void glDisable(GLenum){}
inline void glBlendFunc(GLenum,GLenum){}
inline void glPixelStorei(GLenum,GLint){}
inline void glReadPixels(GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,void*){}
inline void glGenTextures(GLsizei,GLuint*){}
inline void glBindTexture(GLenum,GLuint){}
inline void glTexImage2D(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void*){}
inline void glTexParameteri(GLenum,GLenum,GLint){}
inline void glViewport(GLint,GLint,GLsizei,GLsizei){}
inline int gladLoadGLLoader(void*){return 1;}
#define GL_NO_ERROR 0
inline GLenum glGetError(){return 0;}
