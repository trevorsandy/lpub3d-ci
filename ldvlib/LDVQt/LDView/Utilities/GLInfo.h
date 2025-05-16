///////////////////////////////////////////////////////////////////////////////
// GLInfo.h
// ========
// get GL vendor, version, supported extensions and other states using glGet*
// functions and store them GLInfo struct variable
//
// To get valid OpenGL infos, OpenGL rendering context (RC) must be opened
// before calling GLInfo::getGLInfo(). Otherwise it returns false.
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2005-10-04
// UPDATED: 2013-03-06
// UPDATED: 2017-06-20 by Trevor SANDY (add fbo support details, additional tracked parameters)
//
// Copyright (c) 2005-2013 Song Ho Ahn
///////////////////////////////////////////////////////////////////////////////

#ifndef GLINFO_H
#define GLINFO_H

#include <string>
#include <vector>

// struct variable to store OpenGL info
struct GLInfo
{
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string glslVersion;
    std::vector <std::string> extensions;
    int redBits;
    int greenBits;
    int blueBits;
    int alphaBits;
    int depthBits;
    int stencilBits;
    int maxTextureSize;
    int maxLights;
    int maxAttribStacks;
    int maxModelViewStacks;
    int maxProjectionStacks;
    int maxClipPlanes;
    int maxTextureStacks;
    int maxAnisotropy;
    int maxDrawBuffers;
    int maxColorAttachments;
    bool fboSupported;
    bool wglSwapControl;

    // ctor, init all members
    GLInfo() : redBits(0), greenBits(0), blueBits(0), alphaBits(0),
               depthBits(0), stencilBits(0), maxTextureSize(0), maxLights(0),
               maxAttribStacks(0), maxModelViewStacks(0), maxProjectionStacks(0),
               maxClipPlanes(0), maxTextureStacks(0), maxAnisotropy(0), maxDrawBuffers(0),
               maxColorAttachments(0), fboSupported(false), wglSwapControl(false) {}

    void getGLInfo(unsigned int param=0);                // extract info
    void printGLInfo();                                  // print itself
    bool isExtensionSupported(const std::string& ext); // check if a extension is supported
};

#endif
