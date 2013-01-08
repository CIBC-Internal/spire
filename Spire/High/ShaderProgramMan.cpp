/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
/// \date   January 2013

#include "ShaderProgramMan.h"
#include "Exceptions.h"

namespace Spire {

//------------------------------------------------------------------------------
std::shared_ptr<ShaderProgramAsset>
ShaderProgramMan::loadProgram(const std::string& programName,
                      const std::list<std::tuple<std::string, GLenum>>& shaders)
{
  std::shared_ptr<BaseAsset> asset = findAsset(programName);
  if (asset == nullptr)
  {
    std::shared_ptr<ShaderProgramAsset> program(new ShaderProgramAsset(
            programName, shaders));

    // Add the asset
    addAsset(std::dynamic_pointer_cast<BaseAsset>(program));

    return program;
  }
  else
  {
    return std::dynamic_pointer_cast<ShaderProgramAsset>(asset); 
  }
}

//------------------------------------------------------------------------------
ShaderProgramAsset::ShaderProgramAsset(Hub& hub, const std::string& name,
                      const std::list<std::tuple<std::string, GLenum>>& shaders)
{
  GLuint program = glCreateProgram();
  if (0 == program)
  {
    Log::error() << "Unable to create GL program using glCreateProgram.\n";
    throw GLError("Unable to create shader program.");
  }

  // Load and attach all shaders.
  for (auto it = shaders.begin(); it != shaders.end(); ++it)
  {
    // Attempt to find shader.
    std::shared_ptr<ShaderAsset> shader = 
        mHub.getShaderManager().loadShader(std::get<0>(*it), std::get<1>(*it));

    glAttachShader(program, shader->getShaderID());
  }

  // Now sync up program attributes (uniforms do not play a part in linking).
}

//------------------------------------------------------------------------------
ShaderProgramAsset::~ShaderProgramAsset()
{
}


} } // end of namespace Spire::High
