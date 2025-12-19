#ifndef __CHECKERROR_H__
#define __CHECKERROR_H__

#include <stdio.h>
#include <GL/gl.h>

void CheckError()
{
    GLenum errCode;
    const GLubyte *errString;

    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        printf("OpenGL Error: %s\n", errString);
    }
}

#endif
