#ifndef __TREGL_H__
#define __TREGL_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#else // WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif // WIN32

#ifndef WIN32
//#ifndef __APPLE__
#define GL_GLEXT_PROTOTYPES
//#endif
#endif // WIN32

#ifdef __APPLE__
#  define GL_GLEXT_LEGACY
#  include <GLUT/GLUT.h>
#  define APIENTRY
#  ifdef _OSMESA
#    include <GL/gl.h>
#    include <GL/glext.h>
#    include <GL/glu.h>
#  else // _OSMESA
#    include <OpenGL/gl.h>
#    include "../include/GL/glext.h"
#    include <OpenGL/OpenGL.h>
#  endif // _OSMESA
#else	// __APPLE__
#  include <GL/gl.h>
#  include <GL/glext.h>
#  include <GL/glu.h>
#endif	// __APPLE__

#include <TCFoundation/TCDefines.h>

//Solaris
#if (defined (__SVR4) && defined (__sun)) 
#define APIENTRY
#endif

#ifdef WIN32
#include <GL/wglext.h>
#else // WIN32

#ifndef __APPLE__
#ifndef GL_ARB_vertex_buffer_object
typedef int GLsizeiptrARB;
#endif // !GL_ARB_vertex_buffer_object
#endif // !APPLE

//	WGL ext stuff
typedef void * (APIENTRY * PFNWGLALLOCATEMEMORYNVPROC) (int size, float readfreq, float writefreq, float priority);
typedef void (APIENTRY * PFNWGLFREEMEMORYNVPROC) (void *pointer);

//	ext stuff
typedef void (APIENTRY * PFNGLFLUSHVERTEXARRAYRANGENVPROC) (void);
typedef void (APIENTRY * PFNGLVERTEXARRAYRANGENVPROC) (GLsizei size, const GLvoid *pointer);
typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);

// Qt on macOS automatically loads OpenGL/glext.h (and OpenGL/gl3.h)
// which prevents '../include/GL/glext.h' extensions from loading.
#if defined(__APPLE__) && defined(QT_OPENGL_LIB)
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
// Forced extensions from ../include/GL/glext.h
// GL_EXT_multi_draw_arrays
typedef void (APIENTRYP PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
// GL_ARB_vertex_buffer_object
typedef GLboolean (APIENTRYP PFNGLISBUFFERARBPROC) (GLuint buffer);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
typedef void (APIENTRYP PFNGLGETBUFFERSUBDATAARBPROC) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERARBPROC) (GLenum target, GLenum access);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERARBPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETBUFFERPARAMETERIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETBUFFERPOINTERVARBPROC) (GLenum target, GLenum pname, GLvoid* *params);
// GL_ARB_occlusion_query
typedef void (APIENTRYP PFNGLGENQUERIESARBPROC) (GLsizei n, GLuint *ids);
typedef void (APIENTRYP PFNGLDELETEQUERIESARBPROC) (GLsizei n, const GLuint *ids);
typedef GLboolean (APIENTRYP PFNGLISQUERYARBPROC) (GLuint id);
typedef void (APIENTRYP PFNGLBEGINQUERYARBPROC) (GLenum target, GLuint id);
typedef void (APIENTRYP PFNGLENDQUERYARBPROC) (GLenum target);
typedef void (APIENTRYP PFNGLGETQUERYIVARBPROC) (GLenum target, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTIVARBPROC) (GLuint id, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGETQUERYOBJECTUIVARBPROC) (GLuint id, GLenum pname, GLuint *params);
// GL_EXT_framebuffer_object
typedef GLboolean (APIENTRYP PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
typedef GLboolean (APIENTRYP PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);
#endif // USING_QT ON MACOS

//Solaris
#if (defined (__SVR4) && defined (__sun))
typedef GLvoid (APIENTRY * PFNGLMULTIDRAWELEMENTSEXTPROC) (GLenum mode, GLsizei *count, GLenum type, const GLvoid **indices, GLsizei primcount);
#endif

#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4

#ifndef GL_VERTEX_ARRAY_RANGE_NV
#define GL_VERTEX_ARRAY_RANGE_NV 0x851D
#endif

#endif // WIN32

#ifdef LDVIEW_DOUBLES
#define treGlMultMatrixf glMultMatrixd
#define treGlTranslatef glTranslated
#define treGlVertex3fv glVertex3dv
#define treGlVertex3f glVertex3d
#define treGlGetFloatv glGetDoublev
#define treGlTexCoord2f glTexCoord2d
#define treGlRotatef glRotated
#define TRE_GL_FLOAT GL_DOUBLE
#else // LDVIEW_DOUBLES
#define treGlMultMatrixf glMultMatrixf
#define treGlTranslatef glTranslatef
#define treGlVertex3fv glVertex3fv
#define treGlVertex3f glVertex3f
#define treGlGetFloatv glGetFloatv
#define treGlTexCoord2f glTexCoord2f
#define treGlRotatef glRotatef
#define TRE_GL_FLOAT GL_FLOAT
#endif // LDVIEW_DOUBLES

#endif // __TREGL_H__
