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
/// \date   February 2013

#include <functional>

#include "Common.h"
#include "StuInterface.h"
#include "StuObject.h"
#include "Core/ShaderProgramMan.h"
#include "Core/Hub.h"

using namespace std::placeholders;

namespace Spire {

//------------------------------------------------------------------------------
StuInterface::StuInterface(Interface& iface) :
    PipeInterface(iface)
{
}

//------------------------------------------------------------------------------
StuInterface::~StuInterface()
{
}

//------------------------------------------------------------------------------
void StuInterface::initOnRenderThread()
{
}

//------------------------------------------------------------------------------
void StuInterface::doPass()
{
  
}

//------------------------------------------------------------------------------
void StuInterface::addIBOToObjectImpl(Hub& hub, StuInterface* iface,
                                      std::string object, std::string name,
                                      std::shared_ptr<std::vector<uint8_t>> iboData,
                                      StuInterface::IBO_TYPE type)
{
  // The 'at' function will throw std::out_of_range an exception if object
  // doesn't exist.
  StuObject& obj = iface->mObjects.at(object);
  obj.addIBO(name, iboData, type);
}

//------------------------------------------------------------------------------
void StuInterface::addIBOToObject(const std::string& object,
                                  const std::string& name,
                                  std::shared_ptr<std::vector<uint8_t>> iboData,
                                  IBO_TYPE type)
{
  Hub::RemoteFunction fun =
      std::bind(addIBOToObjectImpl, _1, this, object, name, iboData, type);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addVBOToObjectImpl(Hub& hub, StuInterface* iface,
                                      std::string object, std::string name,
                                      std::shared_ptr<std::vector<uint8_t>> vboData,
                                      std::vector<std::string> attribNames)
{
  StuObject& obj = iface->mObjects.at(object);
  obj.addVBO(name, vboData, attribNames);
}


//------------------------------------------------------------------------------
void StuInterface::addVBOToObject(const std::string& object,
                                  const std::string& name,
                                  std::shared_ptr<std::vector<uint8_t>> vboData,
                                  const std::vector<std::string>& attribNames)
{
  Hub::RemoteFunction fun =
      std::bind(addVBOToObjectImpl, _1, this, object, name, vboData, attribNames);
  mHub.addFunctionToThreadQueue(fun);
}

//------------------------------------------------------------------------------
void StuInterface::addGeomPassToObject(const std::string& object,
                                       const std::string& pass,
                                       const std::string& program,
                                       size_t vboID,
                                       size_t iboID)
{
  /// \todo Turn into a message.
  StuObject& obj = mObjects.at(object);
  obj.addPass(pass, program, vboID, iboID);
}

//------------------------------------------------------------------------------
void StuInterface::addObject(const std::string& object)
{
  /// \todo Turn into a message.
  if (mObjects.find(object) != mObjects.end())
    throw Duplicate("There already exists an object by that name!");

  StuObject obj;
  mObjects[object] = std::move(obj);
}

//------------------------------------------------------------------------------
void StuInterface::addPassUniformInternal(const std::string& object,
                                          const std::string& pass,
                                          const std::string& uniformName,
                                          std::unique_ptr<AbstractUniformStateItem>&& item)
{
  /// \todo Turn into a message.
  StuObject& obj = mObjects.at(object);
  // Move is not necessary, but makes things more clear.
  obj.addPassUniform(pass, uniformName, std::move(item));
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::string& vertexShader,
                                       const std::string& fragmentShader)
{
  /// \todo Turn into a message.
  std::list<std::tuple<std::string, GLenum>> shaders;
  shaders.push_back(make_tuple(vertexShader, GL_VERTEX_SHADER));
  shaders.push_back(make_tuple(fragmentShader, GL_FRAGMENT_SHADER));
  std::shared_ptr<ShaderProgramAsset> shader = 
      mHub.getShaderProgramManager().loadProgram("UniformColor", shaders);
}

//------------------------------------------------------------------------------
void StuInterface::addPersistentShader(const std::string& programName,
                                       const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders)
{

}

//------------------------------------------------------------------------------
void StuInterface::removeGeomPassFromObject(const std::string& object,
                                            const std::string& pass)
{
}

//------------------------------------------------------------------------------
void StuInterface::removeObject(const std::string& object)
{
}

}
