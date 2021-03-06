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

#include "Hub.h"
#include "InterfaceImplementation.h"
#include "SpireObject.h"
#include "Exceptions.h"

/// Remove types as we move away from making spire a one-stop-shop for OpenGL.
/// Spire will only solve one uinque problem in terms of gathering shaders
/// and rendering objects with the appropriate uniforms.

#ifndef USE_OPENGL_ES
  #define GL_HALF_FLOAT_OES GL_FLOAT
#endif


namespace CPM_SPIRE_NS {

// Simple static function to convert from PRIMITIVE_TYPES to GL types.
// Not part of the class due to the return type (interface class should have
// nothing GL specific in them).
GLenum getGLPrimitive(Interface::PRIMITIVE_TYPES type);

//------------------------------------------------------------------------------
InterfaceImplementation::InterfaceImplementation(Hub& hub) :
    mHub(hub)
{}

//------------------------------------------------------------------------------
void InterfaceImplementation::clearGLResources()
{
  mNameToObject.clear();
  mPersistentShaders.clear();
  mVBOMap.clear();
  mIBOMap.clear();
}

//------------------------------------------------------------------------------
std::shared_ptr<SpireObject>
InterfaceImplementation::getObjectWithName(const std::string& name) const
{
  return mNameToObject.at(name);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObject(std::string objectName)
{
  if (mNameToObject.find(objectName) != mNameToObject.end())
    throw Duplicate("There already exists an object by that name!");

  std::shared_ptr<SpireObject> obj = std::shared_ptr<SpireObject>(
      new SpireObject(mHub, objectName));
  mNameToObject[objectName] = obj;
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeObject(std::string objectName)
{
  if (mNameToObject.find(objectName) == mNameToObject.end())
    throw std::range_error("Object to remove does not exist!");

  std::shared_ptr<SpireObject> obj = mNameToObject.at(objectName);
  mNameToObject.erase(objectName);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeAllObjects()
{
  mNameToObject.clear();
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addVBO(std::string vboName,
                                     std::shared_ptr<std::vector<uint8_t>> vboData,
                                     std::vector<std::string> attribNames)
{
  if (mVBOMap.find(vboName) != mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object.");

  mVBOMap.insert(std::make_pair(
          vboName, std::shared_ptr<VBOObject>(
              new VBOObject(vboData, attribNames, mHub.getShaderAttributeManager()))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addConcurrentVBO(
    const std::string& vboName, const uint8_t* vboData, size_t vboSize,
    const std::vector<std::string>& attribNames)
{
  if (mVBOMap.find(vboName) != mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object.");

  mVBOMap.insert(std::make_pair(
          vboName, std::shared_ptr<VBOObject>(
              new VBOObject(vboData, vboSize, attribNames, 
                            mHub.getShaderAttributeManager()))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeVBO(std::string vboName)
{
  size_t numElementsRemoved = mVBOMap.erase(vboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find VBO to remove.");
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addIBO(std::string iboName,
                                     std::shared_ptr<std::vector<uint8_t>> iboData,
                                     Interface::IBO_TYPE type)
{
  if (mIBOMap.find(iboName) != mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object.");

  mIBOMap.insert(std::make_pair(
          iboName, std::shared_ptr<IBOObject>(new IBOObject(iboData, type))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addConcurrentIBO(
    const std::string& iboName, const uint8_t* iboData, size_t iboSize,
    Interface::IBO_TYPE type)
{
  if (mIBOMap.find(iboName) != mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object.");

  mIBOMap.insert(std::make_pair(
          iboName, std::shared_ptr<IBOObject>(new IBOObject(iboData, iboSize, type))));
}

//------------------------------------------------------------------------------
void InterfaceImplementation::removeIBO(std::string iboName)
{
  size_t numElementsRemoved = mIBOMap.erase(iboName);
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find IBO to remove.");
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPassToObject(
    std::string object, std::string program, std::string vboName,
    std::string iboName, Interface::PRIMITIVE_TYPES type, std::string pass,
    std::string parentPass)
{
  std::shared_ptr<SpireObject> obj = mNameToObject.at(object);
  std::shared_ptr<VBOObject> vbo = mVBOMap.at(vboName);
  std::shared_ptr<IBOObject> ibo = mIBOMap.at(iboName);

  // The 'responsiblePass' must exist. It is under this pass in which objects
  // will be rendered.
  std::string responsiblePass = pass;
  if (parentPass.size() > 0)
    responsiblePass = parentPass;

  obj->addPass(pass, program, vbo, ibo, getGLPrimitive(type), parentPass);
}


//------------------------------------------------------------------------------
void InterfaceImplementation::removePassFromObject(std::string object, std::string pass)
{
  std::shared_ptr<SpireObject> obj = mNameToObject.at(object);
  obj->removePass(pass);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectPassUniformConcrete(std::string object, std::string uniformName,
                                                           std::shared_ptr<AbstractUniformStateItem> item,
                                                           std::string pass)
{
  std::shared_ptr<SpireObject> obj = mNameToObject.at(object);
  obj->addPassUniform(pass, uniformName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addObjectGlobalUniformConcrete(std::string objectName,
                                                             std::string uniformName,
                                                             std::shared_ptr<AbstractUniformStateItem> item)
{
  std::shared_ptr<SpireObject> obj = mNameToObject.at(objectName);
  obj->addGlobalUniform(uniformName, item);
}


//------------------------------------------------------------------------------
void InterfaceImplementation::addGlobalUniformConcrete(std::string uniformName,
                                                       std::shared_ptr<AbstractUniformStateItem> item)
{
  // Access uniform state manager and apply/update uniform value.
  mHub.getGlobalUniformStateMan().updateGlobalUniform(uniformName, item);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addShaderAttribute(std::string codeName,
                                                 size_t numComponents, bool normalize, size_t size,
                                                 Interface::DATA_TYPES t)
{
  mHub.getShaderAttributeManager().addAttribute(codeName, numComponents, normalize, size, t);
}

//------------------------------------------------------------------------------
void InterfaceImplementation::addPersistentShader(std::string programName,
                                                  std::vector<std::tuple<std::string, Interface::SHADER_TYPES>> tempShaders)
{
  std::list<std::tuple<std::string, GLenum>> shaders;
  for (auto it = tempShaders.begin(); it != tempShaders.end(); ++it)
  {
    GLenum glType = GL_VERTEX_SHADER;
    switch (std::get<1>(*it))
    {
      case Interface::VERTEX_SHADER:
        glType = GL_VERTEX_SHADER;
        break;

      case Interface::FRAGMENT_SHADER:
        glType = GL_FRAGMENT_SHADER;
        break;

      default:
        throw UnsupportedException("This shader is not supported yet.");
    }
    shaders.push_back(make_tuple(std::get<0>(*it), glType));
  }

  std::shared_ptr<ShaderProgramAsset> shader = 
      mHub.getShaderProgramManager().loadProgram(programName, shaders);

  // Check to make sure we haven't already added this shader.
  for (auto it = mPersistentShaders.begin();
       it != mPersistentShaders.end(); ++it)
  {
    if (shader == *it)
      throw Duplicate("Attempted to add duplicate shader to persistent shader list");
  }
  mPersistentShaders.push_back(shader);
}


//------------------------------------------------------------------------------
GLenum InterfaceImplementation::getGLPrimitive(Interface::PRIMITIVE_TYPES type)
{
  switch (type)
  {
    case Interface::POINTS:                   return GL_POINTS;
    case Interface::LINES:                    return GL_LINES;
    case Interface::LINE_LOOP:                return GL_LINE_LOOP;
    case Interface::LINE_STRIP:               return GL_LINE_STRIP;
    case Interface::TRIANGLES:                return GL_TRIANGLES;
    case Interface::TRIANGLE_STRIP:           return GL_TRIANGLE_STRIP;
    case Interface::TRIANGLE_FAN:             return GL_TRIANGLE_FAN;
    // Adjacency is only supported in OpenGL 4.0 +. Will add when we support
    // Core profiles, starting with core profile 4.1.
#if defined(USE_CORE_PROFILE_4)
    case Interface::LINES_ADJACENCY:          return GL_LINES_ADJACENCY;
    case Interface::LINE_STRIP_ADJACENCY:     return GL_LINE_STRIP_ADJACENCY;
    case Interface::TRIANGLES_ADJACENCY:      return GL_TRIANGLES_ADJACENCY;
    case Interface::TRIANGLE_STRIP_ADJACENCY: return GL_TRIANGLE_STRIP_ADJACENCY;
#else
    case Interface::LINES_ADJACENCY:
    case Interface::LINE_STRIP_ADJACENCY:
    case Interface::TRIANGLES_ADJACENCY:
    case Interface::TRIANGLE_STRIP_ADJACENCY:
      Log::error() << "Adjacency primitive types not supported in OpenGL ES 2.0";
      return GL_TRIANGLES;
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    default:
      {
        std::stringstream stream;
        stream << "Expected type to be one of PRIMITIVE_TYPES, received " << type;
        throw std::invalid_argument(stream.str());
      }
#pragma clang diagnostic pop
  }

  return GL_TRIANGLES;
}

//------------------------------------------------------------------------------
GLenum InterfaceImplementation::getGLType(Interface::DATA_TYPES type)
{
  switch (type)
  {
    case Interface::TYPE_BYTE:        return GL_BYTE;
    case Interface::TYPE_UBYTE:       return GL_UNSIGNED_BYTE;
    case Interface::TYPE_SHORT:       return GL_SHORT;
    case Interface::TYPE_USHORT:      return GL_UNSIGNED_SHORT;
    case Interface::TYPE_INT:         return GL_INT;
    case Interface::TYPE_UINT:        return GL_UNSIGNED_INT;
    case Interface::TYPE_FLOAT:       return GL_FLOAT;
#ifdef SPIRE_OPENGL_ES_2
    case Interface::TYPE_HALFFLOAT:   return GL_HALF_FLOAT_OES;
#else
    case Interface::TYPE_HALFFLOAT:
      Log::error() << "Half-float not supported on non-ES platforms.";
      return GL_FLOAT;
#endif

    // Double type not sepported in OpenGL ES 2.0.
#ifndef SPIRE_OPENGL_ES_2
    case Interface::TYPE_DOUBLE:      return GL_DOUBLE;
#else
    case Interface::TYPE_DOUBLE:
      Log::error() << "Double type not supported on ES 2.0 platforms.";
      return GL_FLOAT;
#endif

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    default:
    {
      std::stringstream stream;
      stream << "Expected type to be one of Interface::DATA_TYPES, received " << type;
      throw std::invalid_argument(stream.str());
    }
#pragma clang diagnostic pop
  }

  return GL_FLOAT;
}

} // namespace CPM_SPIRE_NS

