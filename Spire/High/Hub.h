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

#ifndef SPIRE_HIGH_HUB_H
#define SPIRE_HIGH_HUB_H

#include <iostream>
#include <fstream>
#include <functional>
#include <thread>
#include <atomic>

#include "Interface.h"
#include "High/GPUStateManager.h"

namespace Spire {

class Log;
class PipeDriver;

/// Using thread local storage ONLY for logging purposes, nothing else.

/// Central hub for the renderer.
/// Most managers will reference this class in some way.
class Hub
{
public:

  /// @todo This typedef should go in Interface.h.
  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// @todo Make context a shared_ptr
  Hub(Context* context, LogFunction logFn, bool useThread);
  virtual ~Hub();

  /// One-time initialization of the renderer.
  /// Called by the rendering thread, or the thread where this Interface class
  /// was created (called automatically from interface's constructor in the 
  /// latter case).
  void oneTimeInitOnThread();

  /// If anything in the scene has changed, then calling this will render
  /// a new frame and swap the buffers. If the scene was not modified, then this
  /// function does nothing.
  void doFrame();

  /// Retrieves the GPU state manager.
  GPUStateManager& getGPUStateManager()   {return mGPUStateManager;}

  /// Returns true if the rendering thread is currently running.
  bool isRendererThreadRunning();

private:

  LogFunction                 mLogFun;          ///< Log function.
  std::unique_ptr<Log>        mLog;             ///< Spire logging class.
  Context*                    mContext;         ///< Rendering context.
  GPUStateManager             mGPUStateManager; ///< GPU state manager.
  std::shared_ptr<PipeDriver> mPipe;            ///< Current rendering pipe.


  // Threading variables / functions

  /// Rendering thread function
  void rendererThread();

  /// Terminates the rendering thread. After this call, you will be able to
  /// re-issue context->makeCurrent() and call doFrame manually.
  /// killRendererThread WILL block until the rendering thread has finished.
  /// This is to ensure makeCurrent will not be called again before the thread 
  /// has terminated.
  void killRendererThread();

  /// Creates a rendering thread. 
  /// There must not be a rendering thread already running.
  void createRendererThread();


  std::thread               mThread;        ///< The renderer thread.
  std::atomic<bool>         mThreadKill;    ///< If true, the renderer thread
                                            ///< will attempt to finish what it
                                            ///< is doing and terminate.
  std::atomic<bool>         mThreadRunning; ///< True if the rendering thread
                                            ///< is currently running.
};

} // namespace Spire

#endif // SPIRE_HIGH_HUB_H
