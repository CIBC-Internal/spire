/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2010 Scientific Computing and Imaging Institute,
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

#include <stdexcept>
#include <cassert>
#include "GL/osmesa.h"

#include "OSMesaContext.h"

namespace Spire {

struct MesaContext
{
  MesaContext() :frameBuffer(nullptr) {}

  OSMesaContext ctx;  
  void*         frameBuffer;
  uint32_t      width;
  uint32_t      height;
};

OSMesaBatchContext::OSMesaBatchContext(uint32_t width, uint32_t height, uint8_t,
                                       uint8_t depthBits, uint8_t stencilBits,
                                       bool double_buffer, bool visible) :
  mContext(new MesaContext())
{
  mContext->ctx = OSMesaCreateContextExt( OSMESA_RGBA, depthBits, stencilBits,
                                          0, NULL );

  mContext->frameBuffer = malloc( width * height * 4 * sizeof(GLubyte) );
  if (mContext->frameBuffer == nullptr)
  {
    throw NoAvailableContext();
  }
  mContext->width = width;
  mContext->height = height;

  this->makeCurrent();
}

OSMesaBatchContext::~OSMesaBatchContext()
{
  free(mContext->frameBuffer);
  OSMesaDestroyContext(mContext->ctx);
}

bool OSMesaBatchContext::isValid() const
{
  return mContext->frameBuffer != NULL;
}

void OSMesaBatchContext::makeCurrent()
{
  if ( !OSMesaMakeCurrent( mContext->ctx, mContext->frameBuffer, GL_UNSIGNED_BYTE,
                           mContext->width, mContext->height) )
    throw std::runtime_error("Unable make context current.");
}

void OSMesaBatchContext::swapBuffers()
{
  // We just need to make sure the gl commands are finished.
  glFinish();
}

}
