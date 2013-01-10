/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   September 2012

#include "GLWidget.h"

GLWidget::GLWidget(const QGLFormat& format) :
    QGLWidget(format),
    mContext(this)
{
  std::vector<std::string> shaderSearchDirs = {"Shaders"};

  // Create a threaded spire renderer.
  mGraphics = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(&mContext, shaderSearchDirs, true));

  // We must disable auto buffer swap on the 'paintEvent'.
  setAutoBufferSwap(false);
}

void GLWidget::resizeEvent(QResizeEvent *evt)
{
  /// @todo Inform the renderer that screen dimensions have changed.
  //mGraphics.resizeViewport(evt->size());
}

void GLWidget::closeEvent(QCloseEvent *evt)
{
  // Kill off the graphics thread.
  mGraphics.reset();
  QGLWidget::closeEvent(evt);
}

