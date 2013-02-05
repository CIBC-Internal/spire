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

#include <sstream>
#include "Interface.h"
#include "Exceptions.h"
#include "Core/Hub.h"
#include "Core/Log.h"
#include "Core/InterfaceImplementation.h"
#include "Core/ThreadMessage.h"

namespace Spire {

//------------------------------------------------------------------------------
Interface::Interface(std::shared_ptr<Context> context,
                     const std::vector<std::string>& shaderDirs,
                     bool createThread, LogFunction logFP) :
    mHub(new Hub(context, shaderDirs, logFP, createThread)),
    mInterfaceImpl(new InterfaceImplementation())
{
}

//------------------------------------------------------------------------------
Interface::~Interface()
{
}

//------------------------------------------------------------------------------
void Interface::terminate()
{
  if (mHub->isRendererThreadRunning())
  {
    mHub->killRendererThread();
  }
}

//------------------------------------------------------------------------------
void Interface::doFrame()
{
  if (mHub->isRendererThreadRunning())
    throw ThreadException("You cannot call doFrame when the renderer is "
                          "running in a separate thread.");

  mHub->doFrame();
}

//------------------------------------------------------------------------------
void Interface::cameraSetTransform(const M44& transform)
{
  using namespace std::placeholders;

  // Bind the cameraSetTransform function in the interface implementation.
  ThreadMessage::RemoteFunction fun = 
      std::bind(InterfaceImplementation::cameraSetTransform,
                _1, transform);

  // Now place the remote function in the queue...
}

} // end of namespace Renderer

