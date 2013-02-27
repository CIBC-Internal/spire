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

#include "StuObject.h"

namespace Spire {

//------------------------------------------------------------------------------
// VBO
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
VBOObject::VBOObject(std::shared_ptr<std::vector<uint8_t>> vboData,
            const std::vector<std::string>& attributes)
{
}

//------------------------------------------------------------------------------
VBOObject::~VBOObject()
{
}

//------------------------------------------------------------------------------
// IBO
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
IBOObject::~IBOObject()
{
}

//------------------------------------------------------------------------------
IBOObject::IBOObject(std::shared_ptr<std::vector<uint8_t>> iboData,
            StuInterface::IBO_TYPE type)
{
}

//------------------------------------------------------------------------------
// StuPass
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuPass::StuPass(const std::string& objectName, const std::string& programName,
                 std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo) :
    mVBO(vbo),
    mIBO(ibo)
{
  /// \todo Lookup the shader to use
}

//------------------------------------------------------------------------------
StuPass::~StuPass()
{
}

//------------------------------------------------------------------------------
// StuObject
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
StuObject::StuObject(const std::string& name, int32_t renderOrder) :
    mName(name),
    mRenderOrder(renderOrder)
{

}

//------------------------------------------------------------------------------
void StuObject::addIBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> iboData,
              StuInterface::IBO_TYPE type)
{
  size_t hash = mHashFun(name);
  if (mIBOMap.find(hash) != mIBOMap.end())
    throw Duplicate("Attempting to add duplicate IBO to object (possible hash collision?).");

  mIBOMap.insert(std::make_pair(
          hash, std::shared_ptr<IBOObject>(new IBOObject(iboData, type))));
}

//------------------------------------------------------------------------------
void StuObject::removeIBO(const std::string& name)
{
  size_t numElementsRemoved = mIBOMap.erase(mHashFun(name));
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find IBO to remove.");
}

//------------------------------------------------------------------------------
void StuObject::addPass(
    const std::string& passName,
    const std::string& program,
    const std::string& vboName,
    const std::string& iboName)
{
  // Check to see if there already is a pass by that name...
  if (mPasses.find(passName) != mPasses.end())
    throw Duplicate("There already exists a pass with the specified pass name.");

  // Build the pass.

}

//------------------------------------------------------------------------------
void StuObject::addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item)
{
}

//------------------------------------------------------------------------------
void StuObject::addVBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> vboData,
              const std::vector<std::string>& attribNames)
{
  size_t hash = mHashFun(name);
  if (mVBOMap.find(hash) != mVBOMap.end())
    throw Duplicate("Attempting to add duplicate VBO to object (possible hash collision?).");

  mVBOMap.emplace(std::make_pair(
          hash, std::shared_ptr<VBOObject>(new VBOObject(vboData, attribNames))));
}

//------------------------------------------------------------------------------
void StuObject::removeVBO(const std::string& vboName)
{
  size_t numElementsRemoved = mIBOMap.erase(mHashFun(vboName));
  if (numElementsRemoved == 0)
    throw std::out_of_range("Could not find VBO to remove.");
}

//------------------------------------------------------------------------------
void StuObject::removePass(const std::string& pass)
{
}

//------------------------------------------------------------------------------
std::shared_ptr<IBOObject> StuObject::getIBOByName(const std::string& name)
{
  size_t hash = mHashFun(name);
  return mIBOMap.at(hash);
}

//------------------------------------------------------------------------------
std::shared_ptr<VBOObject> StuObject::getVBOByName(const std::string& name)
{
  size_t hash = mHashFun(name);
  return mVBOMap.at(hash);
}


} // end of namespace Spire
