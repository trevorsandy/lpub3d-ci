///////////////////////////////////////////////////////////////////////////////
// glInfo.cpp
// ==========
// get GL vendor, version, supported extensions and other states using glGet*
// functions and store them glInfo struct variable
//
// To get valid OpenGL infos, OpenGL rendering context (RC) must be opened
// before calling GLInfo::getGLInfo(). Otherwise it returns false.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
//          https://www.songho.ca/opengl/index.html
// CREATED: 2005-10-04
// UPDATED: 2013-03-06
// UPDATED: 2017-06-20 by Trevor SANDY (add fbo support details, additional tracked parameters)
//
// Copyright (c) 2005-2013 Song Ho Ahn
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#  include <GLUT/GLUT.h>
#  ifdef _OSMESA
#    include <GL/gl.h>
#    include <GL/glext.h>
#  else // _OSMESA
#    include <OpenGL/gl.h>
#    include "../include/GL/glext.h"
#  endif // _OSMESA
#else   // __APPLE__
#  include <GL/gl.h>
#  include <GL/glext.h>
#endif  // __APPLE__
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include "GLInfo.h"


// WGL specific extensions for v3.0+ //////////////////////////////////////////
#ifdef _WIN32
//#include <windows.h>
#ifndef WGLGETEXTENSIONSSTRINGARB_DEF
#define WGLGETEXTENSIONSSTRINGARB_DEF
typedef const char* (WINAPI * PFNWGLGETEXTENSIONSSTRINGARBPROC)(HDC hdc);
PFNWGLGETEXTENSIONSSTRINGARBPROC    pwglGetExtensionsStringARB = 0;
#define wglGetExtensionsStringARB  pwglGetExtensionsStringARB
#endif


// function pointers for FBO extension
// Windows needs to get function pointers from ICD OpenGL drivers,
// because opengl32.dll does not support extensions higher than v1.1.

// ARB Framebuffer object
PFNGLGENFRAMEBUFFERSPROC                        pglGenFramebuffers = 0;                      // FBO name generation procedure
PFNGLDELETEFRAMEBUFFERSPROC                     pglDeleteFramebuffers = 0;                   // FBO deletion procedure
PFNGLBINDFRAMEBUFFERPROC                        pglBindFramebuffer = 0;                      // FBO bind procedure
PFNGLCHECKFRAMEBUFFERSTATUSPROC                 pglCheckFramebufferStatus = 0;               // FBO completeness test procedure
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC    pglGetFramebufferAttachmentParameteriv = 0;  // return various FBO parameters
PFNGLGENERATEMIPMAPPROC                         pglGenerateMipmap = 0;                       // FBO automatic mipmap generation procedure
PFNGLFRAMEBUFFERTEXTURE1DPROC                   pglFramebufferTexture1D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURE2DPROC                   pglFramebufferTexture2D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURE3DPROC                   pglFramebufferTexture3D = 0;                 // FBO texdture attachement procedure
PFNGLFRAMEBUFFERTEXTURELAYERPROC                pglFramebufferTextureLayer = 0;              // FBO texdture attachement procedure
PFNGLFRAMEBUFFERRENDERBUFFERPROC                pglFramebufferRenderbuffer = 0;              // FBO renderbuffer attachement procedure
PFNGLISFRAMEBUFFERPROC                          pglIsFramebuffer = 0;                        // FBO state = true/false
PFNGLBLITFRAMEBUFFERPROC                        pglBlitFramebuffer = 0;                      // FBO copy
// Renderbuffer object
PFNGLGENRENDERBUFFERSPROC                       pglGenRenderbuffers = 0;                     // renderbuffer generation procedure
PFNGLDELETERENDERBUFFERSPROC                    pglDeleteRenderbuffers = 0;                  // renderbuffer deletion procedure
PFNGLBINDRENDERBUFFERPROC                       pglBindRenderbuffer = 0;                     // renderbuffer bind procedure
PFNGLRENDERBUFFERSTORAGEPROC                    pglRenderbufferStorage = 0;                  // renderbuffer memory allocation procedure
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC         pglRenderbufferStorageMultisample = 0;       // renderbuffer memory allocation with multisample
PFNGLGETRENDERBUFFERPARAMETERIVPROC             pglGetRenderbufferParameteriv = 0;           // return various renderbuffer parameters
PFNGLISRENDERBUFFERPROC                         pglIsRenderbuffer = 0;                       // determine renderbuffer object type

#define glGenFramebuffers                        pglGenFramebuffers
#define glDeleteFramebuffers                     pglDeleteFramebuffers
#define glBindFramebuffer                        pglBindFramebuffer
#define glCheckFramebufferStatus                 pglCheckFramebufferStatus
#define glGetFramebufferAttachmentParameteriv    pglGetFramebufferAttachmentParameteriv
#define glGenerateMipmap                         pglGenerateMipmap
#define glFramebufferTexture1D                   pglFramebufferTexture1D
#define glFramebufferTexture2D                   pglFramebufferTexture2D
#define glFramebufferTexture3D                   pglFramebufferTexture3D
#define glFramebufferTextureLayer                pglFramebufferTextureLayer
#define glFramebufferRenderbuffer                pglFramebufferRenderbuffer
#define glIsFramebuffer                          pglIsFramebuffer
#define glBlitFramebuffer                        pglBlitFramebuffer

#define glGenRenderbuffers                       pglGenRenderbuffers
#define glDeleteRenderbuffers                    pglDeleteRenderbuffers
#define glBindRenderbuffer                       pglBindRenderbuffer
#define glRenderbufferStorage                    pglRenderbufferStorage
#define glRenderbufferStorageMultisample         pglRenderbufferStorageMultisample
#define glGetRenderbufferParameteriv             pglGetRenderbufferParameteriv
#define glIsRenderbuffer                         pglIsRenderbuffer


// function pointers for WGL_EXT_swap_control
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC) (void);
PFNWGLSWAPINTERVALEXTPROC pwglSwapIntervalEXT = 0;
PFNWGLGETSWAPINTERVALEXTPROC pwglGetSwapIntervalEXT = 0;
#define wglSwapIntervalEXT      pwglSwapIntervalEXT
#define wglGetSwapIntervalEXT   pwglGetSwapIntervalEXT
#endif

// version 2.0 or greater
#define GL_SHADING_LANGUAGE_VERSION       0x8B8C

#define INT2HDCPTR(i) (HDC)(uintptr_t)(i)

///////////////////////////////////////////////////////////////////////////////
// extract openGL info
// This function must be called after GL rendering context opened.
///////////////////////////////////////////////////////////////////////////////
void GLInfo::getGLInfo(unsigned int param)
{
    std::string str;

    // get vendor string
    str = (const char*)glGetString(GL_VENDOR);
    this->vendor = str;             // check NULL return value

    // get renderer string
    str = (const char*)glGetString(GL_RENDERER);
    this->renderer = str;           // check NULL return value

    // get version string
    str = (const char*)glGetString(GL_VERSION);
    this->version = str;            // check NULL return value

    // get all extensions as a string
    str = (const char*)glGetString(GL_EXTENSIONS);

    // split extensions
    if(str.size() > 0)
    {
        char* str2 = new char[str.size() + 1];
        strcpy(str2, str.c_str());
        char* tok = strtok(str2, " ");
        while(tok)
        {
            this->extensions.push_back(tok);    // put a extension into struct
            tok = strtok(0, " ");               // next token
        }
        delete [] str2;
    }

    // get GLSL version string (v2.0+)
    str = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    if(str.size() > 0)
        this->glslVersion = str;
    else
    {
        // "GL_SHADING_LANGUAGE_VERSION" token is added later (v2.0) after the
        // first GLSL included in OpenGL (v1.5). If "GL_SHADING_LANGUAGE_VERSION"
        // is invalid token but "GL_ARB_shading_language_100" is supported, then
        // the GLSL version should be 1.0.rev.51
        if(isExtensionSupported("GL_ARB_shading_language_100"))
            glslVersion = "1.0.51"; // the first GLSL version
        else
            glslVersion = "";
    }



    // get WGL specific extensions for v3.0+
#ifdef _WIN32 //===========================================
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if(wglGetExtensionsStringARB && param)
    {
        str = (const char*)wglGetExtensionsStringARB(INT2HDCPTR(param));
        if(str.size() > 0)
        {
            char* str2 = new char[str.size() + 1];
            strcpy(str2, str.c_str());
            char* tok = strtok(str2, " ");
            while(tok)
            {
                this->extensions.push_back(tok);    // put a extension into struct
                tok = strtok(0, " ");               // next token
            }
            delete [] str2;
        }
    }
#endif //==================================================

    // sort extension by alphabetical order
    std::sort(this->extensions.begin(), this->extensions.end());

    // get number of color bits
    glGetIntegerv(GL_RED_BITS, &this->redBits);
    glGetIntegerv(GL_GREEN_BITS, &this->greenBits);
    glGetIntegerv(GL_BLUE_BITS, &this->blueBits);
    glGetIntegerv(GL_ALPHA_BITS, &this->alphaBits);

    // get depth bits
    glGetIntegerv(GL_DEPTH_BITS, &this->depthBits);

    // get stecil bits
    glGetIntegerv(GL_STENCIL_BITS, &this->stencilBits);

    // get max number of lights allowed
    glGetIntegerv(GL_MAX_LIGHTS, &this->maxLights);

    // get max texture resolution
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &this->maxTextureSize);

    // get max number of clipping planes
    glGetIntegerv(GL_MAX_CLIP_PLANES, &this->maxClipPlanes);

    // get max modelview and projection matrix stacks
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &this->maxModelViewStacks);
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &this->maxProjectionStacks);
    glGetIntegerv(GL_MAX_ATTRIB_STACK_DEPTH, &this->maxAttribStacks);

    // get max texture stacks
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &this->maxTextureStacks);
    glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &this->maxAnisotropy);

#ifdef _WIN32
    // check if FBO is supported by your video card
    if(isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        // get pointers to GL functions
        glGenFramebuffers                     = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
        glDeleteFramebuffers                  = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
        glBindFramebuffer                     = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
        glCheckFramebufferStatus              = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
        glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)wglGetProcAddress("glGetFramebufferAttachmentParameteriv");
        glGenerateMipmap                      = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
        glFramebufferTexture1D                = (PFNGLFRAMEBUFFERTEXTURE1DPROC)wglGetProcAddress("glFramebufferTexture1D");
        glFramebufferTexture2D                = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
        glFramebufferTexture3D                = (PFNGLFRAMEBUFFERTEXTURE3DPROC)wglGetProcAddress("glFramebufferTexture3D");
        glFramebufferTextureLayer             = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)wglGetProcAddress("glFramebufferTextureLayer");
        glFramebufferRenderbuffer             = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        glIsFramebuffer                       = (PFNGLISFRAMEBUFFERPROC)wglGetProcAddress("glIsFramebuffer");
        glBlitFramebuffer                     = (PFNGLBLITFRAMEBUFFERPROC)wglGetProcAddress("glBlitFramebuffer");
        glGenRenderbuffers                    = (PFNGLGENRENDERBUFFERSPROC)wglGetProcAddress("glGenRenderbuffers");
        glDeleteRenderbuffers                 = (PFNGLDELETERENDERBUFFERSPROC)wglGetProcAddress("glDeleteRenderbuffers");
        glBindRenderbuffer                    = (PFNGLBINDRENDERBUFFERPROC)wglGetProcAddress("glBindRenderbuffer");
        glRenderbufferStorage                 = (PFNGLRENDERBUFFERSTORAGEPROC)wglGetProcAddress("glRenderbufferStorage");
        glRenderbufferStorageMultisample      = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)wglGetProcAddress("glRenderbufferStorageMultisample");
        glGetRenderbufferParameteriv          = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)wglGetProcAddress("glGetRenderbufferParameteriv");
        glIsRenderbuffer                      = (PFNGLISRENDERBUFFERPROC)wglGetProcAddress("glIsRenderbuffer");

        // check once again FBO extension
        if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glCheckFramebufferStatus &&
           glGetFramebufferAttachmentParameteriv && glGenerateMipmap && glFramebufferTexture1D && glFramebufferTexture2D && glFramebufferTexture3D &&
           glFramebufferTextureLayer && glFramebufferRenderbuffer && glIsFramebuffer && glBlitFramebuffer &&
           glGenRenderbuffers && glDeleteRenderbuffers && glBindRenderbuffer && glRenderbufferStorage &&
           glRenderbufferStorageMultisample && glGetRenderbufferParameteriv && glIsRenderbuffer)
        {
            fboSupported = true;
            // get max color attachments and draw buffers
            glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &this->maxColorAttachments);
            glGetIntegerv(GL_MAX_DRAW_BUFFERS, &this->maxDrawBuffers);
        }
    }

    // check EXT_swap_control is supported
    if(isExtensionSupported("WGL_EXT_swap_control"))
    {
        // get pointers to WGL functions
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
        if(wglSwapIntervalEXT && wglGetSwapIntervalEXT)
        {
            wglSwapControl = true;
        }
    }

#else // for linux, do not need to get function pointers, it is up-to-date
    if(isExtensionSupported("GL_ARB_framebuffer_object"))
    {
        fboSupported = true;
        // get max color attachments and draw buffers
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &this->maxColorAttachments);
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &this->maxDrawBuffers);
    }

#endif
}



///////////////////////////////////////////////////////////////////////////////
// check if the video card support a certain extension
///////////////////////////////////////////////////////////////////////////////
bool GLInfo::isExtensionSupported(const std::string& ext)
{
    // search corresponding extension
    std::vector<std::string>::const_iterator iter = this->extensions.begin();
    std::vector<std::string>::const_iterator endIter = this->extensions.end();

    while(iter != endIter)
    {
        if(ext == *iter)
            return true;
        else
            ++iter;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// print OpenGL info to screen and save to a file
///////////////////////////////////////////////////////////////////////////////
void GLInfo::printGLInfo()
{
    getGLInfo();

    std::stringstream ss;

    ss << std::endl; // blank line
    ss << "OpenGL Driver Info" << std::endl;
    ss << "==========================" << std::endl;
    ss << "Vendor: " << this->vendor << std::endl;
    ss << "Version: " << this->version << std::endl;
    ss << "GLSL Version: " << this->glslVersion << std::endl;
    ss << "Renderer: " << this->renderer << std::endl;

    ss << std::endl; // blank line
    ss << "OpenGL Framebuffer" << std::endl;
    ss << "==========================" << std::endl;
    if (fboSupported)
    {
        ss << "GL_ARB Framebuffer Object supported" << std::endl;
    }
    else
    {
        ss << "GL_ARB Framebuffer Object NOT supported" << std::endl;
    }
#ifdef _WIN32
    if (wglSwapControl)
    {
        ss << "WGL Swap Control supported" << std::endl;
    }
    else
    {
        ss << "WGL Swap Control NOT supported" << std::endl;
    }

#endif
    ss << std::endl; // blank line
    ss << "OpenGL Bits" << std::endl;
    ss << "==========================" << std::endl;
    ss << "Color Bits(R,G,B,A): (" << this->redBits << ", " << this->greenBits
       << ", " << this->blueBits << ", " << this->alphaBits << ")\n";
    ss << "Depth Bits: " << this->depthBits << std::endl;
    ss << "Stencil Bits: " << this->stencilBits << std::endl;

    ss << std::endl; // blank line
    ss << "OpenGL Max Parameters" << std::endl;
    ss << "==========================" << std::endl;
    ss << "Max Texture Size: " << this->maxTextureSize << "x" << this->maxTextureSize << std::endl;
    ss << "Max Lights: " << this->maxLights << std::endl;
    ss << "Max Clip Planes: " << this->maxClipPlanes << std::endl;
    ss << "Max Modelview Matrix Stacks: " << this->maxModelViewStacks << std::endl;
    ss << "Max Projection Matrix Stacks: " << this->maxProjectionStacks << std::endl;
    ss << "Max Attribute Stacks: " << this->maxAttribStacks << std::endl;
    ss << "Max Texture Stacks: " << this->maxTextureStacks << std::endl;
    ss << "Max Texture Filter Anisotropic: " << this->maxAnisotropy << std::endl;
    if (fboSupported)
    {
        ss << "Max Color Attachments: " << this->maxColorAttachments << std::endl;
        ss << "Max Draw Buffers: " << this->maxDrawBuffers << std::endl;
    }
    ss << std::endl; // blank line
    ss << "OpenGL Extensions" << std::endl;
    ss << "Number of Extensions: " << this->extensions.size() << std::endl;
    ss << "==========================" << std::endl;
    for(unsigned int i = 0; i < this->extensions.size(); ++i)
        ss << this->extensions.at(i) << std::endl;

    ss << "==========================" << std::endl;

    std::cout << ss.str() /* << std::endl */;
}
