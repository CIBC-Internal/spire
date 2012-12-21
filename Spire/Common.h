/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/// \author James Hughes
/// \date   December 2012

#ifndef SPIRE_COMMON_H
#define SPIRE_COMMON_H

#include <cstddef>

// OpenGL headers
#ifdef USING_OSX
#include <OpenGL/gl.h>
#elif USING_IOS
#define OPENGL_ES 1
//#elif USING_LINUX
//#elif USING_WINDOWS
#else
#error OpenGL headers not defined for this platform.
#endif

// Utility definitions for non-ES OpenGL implementations.
#ifndef OPENGL_ES
#define GL_HALF_FLOAT_OES GL_FLOAT
#endif

namespace Spire
{

#ifdef _DEBUG
# define GL_CHECK()                                                    \
  do {                                                                 \
    GLenum glerr;                                                      \
    unsigned int iCounter = 0;                                         \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      T_ERROR("GL error before line %u (%s): %s (%#x)",                \
              __LINE__, __FILE__,                                      \
              gluErrorString(glerr),                                   \
              static_cast<unsigned>(glerr));                           \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
  } while(0)
#else
# define GL_CHECK() 
#endif

} // namespace Spire

#endif