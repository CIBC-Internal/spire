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

#ifndef SPIRE_STUPIPE_STUOBJECT_H
#define SPIRE_STUPIPE_STUOBJECT_H

#include <string>
#include <list>
#include <map>

#include "Common.h"
#include "ShaderProgramMan.h"
#include "ShaderUniformStateManTemplates.h"

#include "VBOObject.h"
#include "IBOObject.h"

namespace CPM_SPIRE_NS {

//------------------------------------------------------------------------------
// ObjectPassobject
//------------------------------------------------------------------------------
/// Holds all information regarding specific uniforms for use in the pass
/// and also the GL indices of the VBO / IBO to use.
class ObjectPass
{
public:
  ObjectPass(
      Hub& hub,
      const std::string& passName, const std::string& programName,
      std::shared_ptr<VBOObject> vbo, std::shared_ptr<IBOObject> ibo, GLenum primitiveType);
  virtual ~ObjectPass();
  
  void renderPass();

  const std::string& getName() const    {return mName;}
  GLenum getPrimitiveType() const       {return mPrimitiveType;}

  /// Adds a local uniform to the pass.
  /// throws std::out_of_range if 'uniformName' is not found in the shader's
  /// uniform list.
  bool addPassUniform(const std::string& uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item,
                      bool isObjectGlobalUniform);

  /// Returns an empty shared pointer if no item is present (optional would be
  /// better.
  std::shared_ptr<const AbstractUniformStateItem>
  getPassUniform(const std::string& uniformName);

  /// This function will *not* return true if the uniform was added via the
  /// global object uniforms.
  bool hasPassSpecificUniform(const std::string& uniformName) const;

  /// Unlike the function above, this will return true whether or not object
  /// global uniforms were used to populate the uniform.
  bool hasUniform(const std::string& uniformName) const;

  /// Get unsatisfied uniforms.
  std::vector<Interface::UnsatisfiedUniform> getUnsatisfiedUniforms();

protected:

  struct UniformItem
  {
    UniformItem(const std::string& name,
                std::shared_ptr<AbstractUniformStateItem> uniformItem,
                GLint location, bool passSpecificIn) :
        uniformName(name),
        item(uniformItem),
        shaderLocation(location),
        passSpecific(passSpecificIn)
    {}

    std::string                               uniformName;
    std::shared_ptr<AbstractUniformStateItem> item;
    GLint                                     shaderLocation;
    bool                                      passSpecific;   ///< If true, global uniforms do not overwrite.
  };

  std::string                           mName;      ///< Simple pass name.
  GLenum                                mPrimitiveType;

  /// List of unsatisfied uniforms (the list of uniforms that are not covered
  /// by our mUniforms list).
  /// The set of unsatisfied uniforms should be a subset of the global
  /// uniform state. Otherwise the shader cannot be properly satisfied and a
  /// runtime exception will be thrown.
  /// This list is updated everytime we add or remove elements from mUniforms.
  std::vector<Interface::UnsatisfiedUniform>  mUnsatisfiedUniforms;
  std::vector<UniformItem>              mUniforms;  ///< Local uniforms

  std::shared_ptr<VBOObject>            mVBO;     ///< ID of VBO to use during pass.
  std::shared_ptr<IBOObject>            mIBO;     ///< ID of IBO to use during pass.

  std::shared_ptr<ShaderProgramAsset>   mShader;  ///< Shader to be used when rendering this pass.

  Hub&                                  mHub;     ///< Hub.

};

//------------------------------------------------------------------------------
// SpireObject
//------------------------------------------------------------------------------
class SpireObject
{
public:

  SpireObject(Hub& hub, const std::string& name);

  std::string getName() const     {return mName;}

  /// Adds a geometry pass with the specified index / vertex buffer objects.
  void addPass(const std::string& pass,
               const std::string& program,
               std::shared_ptr<VBOObject> vbo,
               std::shared_ptr<IBOObject> ibo,
               GLenum primType,
               const std::string& parentPass);

  /// \note If we add ability to remove IBOs and VBOs, the IBOs and VBOs will
  ///       not be removed until their corresponding passes are removed
  ///       as well due to the shared_ptr.

  /// Removes a geometry pass from the object.
  void removePass(const std::string& pass);

  // The precedence for uniforms goes: pass -> uniform -> global.
  // So pass is checked first, then the uniform level of uniforms, then the
  // global level of uniforms.
  // Currently the only 'pass' and 'global' are implemented.

  /// Adds a uniform to the pass.
  void addPassUniform(const std::string& pass,
                      const std::string uniformName,
                      std::shared_ptr<AbstractUniformStateItem> item);

  /// Adds a uniform to the pass.
  void addGlobalUniform(const std::string& uniformName,
                        std::shared_ptr<AbstractUniformStateItem> item);

  std::shared_ptr<const AbstractUniformStateItem>
      getPassUniform(const std::string& passName, const std::string& uniformName);

  std::shared_ptr<const AbstractUniformStateItem>
      getGlobalUniform(const std::string& uniformName);

  bool hasPassRenderingOrder(const std::vector<std::string>& passes) const;

  /// \todo Ability to render a single named pass. See github issue #15.
  void renderPass(const std::string& pass);

  /// Returns the associated pass. Otherwise an empty shared_ptr is returned.
  std::shared_ptr<const ObjectPass> getObjectPassParams(const std::string& passName) const;

  /// Returns the number of registered passes.
  size_t getNumPasses() const {return mPasses.size();}

  /// Returns true if there exists a object global uniform with the name
  /// 'uniformName'.
  bool hasGlobalUniform(const std::string& uniformName) const;

  /// Get unsatisfied uniforms for pass.
  std::vector<Interface::UnsatisfiedUniform> getUnsatisfiedUniforms(const std::string& pass);

protected:

  typedef std::shared_ptr<AbstractUniformStateItem> ObjectUniformItem;

  struct ObjectGlobalUniformItem
  {
    ObjectGlobalUniformItem(const std::string& name,
                            std::shared_ptr<AbstractUniformStateItem> uniformItem) :
        uniformName(name),
        item(uniformItem)
    {}

    std::string         uniformName;
    ObjectUniformItem   item;
  };

  struct ObjectPassInternal
  {
    ObjectPassInternal() { }
    ObjectPassInternal(std::shared_ptr<ObjectPass> objectPassIn)
    {
      this->objectPass = objectPassIn;
    }

    /// Pointer to the actual object pass.
    std::shared_ptr<ObjectPass>               objectPass;
    
    /// Pointer to the optional subpasses associated with this object pass.
    std::shared_ptr<std::vector<std::shared_ptr<ObjectPass>>>  objectSubPasses;
  };

  /// Retrieves the pass by name.
  std::shared_ptr<ObjectPass> getPassByName(const std::string& name) const;

  /// All registered passes.
  std::unordered_map<std::string, ObjectPassInternal>   mPasses;
  std::vector<ObjectGlobalUniformItem>                  mObjectGlobalUniforms;

  // These maps may actually be more efficient implemented as an array. The map 
  // sizes are small and cache coherency will be more important. Ignoring for 
  // now until we identify an actual performance bottlenecks.
  // size_t represents a std::hash of a string.
  std::hash<std::string>                        mHashFun;
  std::string                                   mName;

  Hub&                                          mHub;
};


} // namespace CPM_SPIRE_NS

#endif 
