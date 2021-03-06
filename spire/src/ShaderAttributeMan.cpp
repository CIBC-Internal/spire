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

#include <algorithm>
#include <iostream>

#include "Common.h"
#include "Exceptions.h"
#include "InterfaceImplementation.h"

#include "ShaderAttributeMan.h"
#include "ShaderProgramMan.h"

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
ShaderAttributeMan::ShaderAttributeMan()
{
  // Unknown attribute (attribute at 0 index).
  addAttribute(getUnknownName(), 1, false, sizeof(float), Interface::TYPE_FLOAT);
}

//------------------------------------------------------------------------------
ShaderAttributeMan::~ShaderAttributeMan()
{
}

//------------------------------------------------------------------------------
void ShaderAttributeMan::addAttribute(const std::string& codeName,
                                      size_t numComponents, bool normalize,
                                      size_t size, Interface::DATA_TYPES type)
{
  AttribState attrib;
  attrib.index          = mAttributes.size();
  attrib.codeName       = codeName;
  attrib.numComponents  = numComponents;
  attrib.normalize      = normalize;
  attrib.size           = size;
  attrib.type           = type;
  attrib.nameHash       = hashString(codeName);

  mAttributes.push_back(attrib);
}

//------------------------------------------------------------------------------
std::tuple<bool, size_t> 
ShaderAttributeMan::findAttributeWithName(const std::string& codeName) const
{
  // Hash the string, search for the hash, then proceed with string compares
  // to check for collisions.
  size_t targetHash = hashString(codeName);
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (it->nameHash == targetHash)
    {
      // Check for hash collisions
      if (codeName == it->codeName)
      {
        // We have found the attribute, return it.
        return std::make_tuple(true, it->index);
      }
    }
  }

  return std::make_tuple(false, 0);
}

//------------------------------------------------------------------------------
AttribState 
ShaderAttributeMan::getAttributeWithName(const std::string& codeName) const
{
  std::tuple<bool, size_t> attIndex = findAttributeWithName(codeName);
  if (std::get<0>(attIndex) == false)
    throw NotFound("Unable to find attribute with name.");

  return getAttributeAtIndex(std::get<1>(attIndex));
}

//------------------------------------------------------------------------------
size_t ShaderAttributeMan::hashString(const std::string& str)
{
  return std::hash<std::string>()(str);
}

//------------------------------------------------------------------------------
AttribState ShaderAttributeMan::getAttributeAtIndex(size_t index) const
{
  if (index >= mAttributes.size())
    throw std::range_error("Index greater than size of mAttributes.");

  return mAttributes[index];
}

//------------------------------------------------------------------------------
// SHADER ATTRIBUTES
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
AttribState ShaderAttributeCollection::getAttribute(size_t index) const
{
  if (index >= mAttributes.size())
    throw std::range_error("Index greater than size of mAttributes.");

  return mAttributes[index];
}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::getNumAttributes() const
{
  return mAttributes.size();
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::hasAttribute(const std::string& attribName) const
{
  size_t hash = ShaderAttributeMan::hashString(attribName);

  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    AttribState state = *it;
    if (state.nameHash == hash)
    {
      // Check for hash collisions
      if (attribName == state.codeName)
      {
        return true;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::doesSatisfyShader(const ShaderAttributeCollection& compare) const
{
  // Not possible to satisfy shader if there are any unknown attributes.
  if (    compare.hasIndex(ShaderAttributeMan::getUnknownAttributeIndex())
      ||  hasIndex(ShaderAttributeMan::getUnknownAttributeIndex()))
    return false;

  // Compare number of common attributes and the size of our attribute array.
  size_t numCommonAttribs = calculateNumCommonAttributes(compare);
  return (numCommonAttribs == mAttributes.size());
}


//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::calculateStride() const
{
  size_t stride = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    stride += getFullAttributeSize(*it);
  }

  return stride;
}

//------------------------------------------------------------------------------
void ShaderAttributeCollection::addAttribute(const std::string& attribName)
{
  std::tuple<bool,size_t> ret = mAttributeMan.findAttributeWithName(attribName);
  if (std::get<0>(ret))
  {
    AttribState attribData = mAttributeMan.getAttributeAtIndex(std::get<1>(ret));
    mAttributes.push_back(attribData);
  }
  else
  {
    // We did not find the attribute in the attribute manager.
    throw ShaderAttributeNotFound(attribName);
  }
}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::getFullAttributeSize(const AttribState& att) const
{
  return att.size;
}

//------------------------------------------------------------------------------
void ShaderAttributeCollection::bindAttributes(std::shared_ptr<ShaderProgramAsset> program) const
{
  GLsizei stride = static_cast<GLsizei>(calculateStride());
  size_t offset = 0;
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (program->getAttributes().hasAttribute(it->codeName))
    {
      if (it->index != ShaderAttributeMan::getUnknownAttributeIndex())
      {
        AttribState attrib = *it;
        GLint attribPos = glGetAttribLocation(program->getProgramID(), attrib.codeName.c_str());
        GL(glEnableVertexAttribArray(static_cast<GLuint>(attribPos)));
        //Log::debug() << "Binding attribute " << attribPos << " with name '" << attrib.codeName << "' "
        //             << "with num components " << attrib.numComponents << " type " << attrib.type
        //             << " normalize " << attrib.normalize << " and stride: " << stride << std::endl;
        GL(glVertexAttribPointer(static_cast<GLuint>(attribPos),
                                 static_cast<GLint>(attrib.numComponents),
                                 InterfaceImplementation::getGLType(attrib.type), 
                                 static_cast<GLboolean>(attrib.normalize),
                                 stride, reinterpret_cast<const void*>(offset)));
      }
    }

    offset += it->size;
  }
}

//------------------------------------------------------------------------------
void ShaderAttributeCollection::unbindAttributes(std::shared_ptr<ShaderProgramAsset> program) const
{
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    /// \todo Make this check more efficient.
    if (program->getAttributes().hasAttribute(it->codeName))
    {
      if (it->index != ShaderAttributeMan::getUnknownAttributeIndex())
      {
        AttribState attrib = *it;
        GLint attribPos = glGetAttribLocation(program->getProgramID(), attrib.codeName.c_str());
        GL(glDisableVertexAttribArray(static_cast<GLuint>(attribPos)));
      }
    }
  }

}

//------------------------------------------------------------------------------
size_t ShaderAttributeCollection::calculateNumCommonAttributes(const ShaderAttributeCollection& compare) const
{
  size_t numCommon = 0;

  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (compare.hasIndex(it->index))
      ++numCommon;
  }

  return numCommon;
}

//------------------------------------------------------------------------------
bool ShaderAttributeCollection::hasIndex(size_t targetIndex) const
{
  // Could perform a binary search here...
  for (auto it = mAttributes.begin(); it != mAttributes.end(); ++it)
  {
    if (targetIndex == it->index)
      return true;
  }

  return false;
}

} // namespace CPM_SPIRE_NS

