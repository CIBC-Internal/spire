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

#ifndef SPIRE_STUPIPE_STUINTERFACE_H
#define SPIRE_STUPIPE_STUINTERFACE_H

#include <utility>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <map>
#include <tuple>
#include <cstdint>
#include "../Core/PipeInterface.h"
#include "../Core/ShaderUniformStateManTemplates.h"
#include "../Core/GPUStateManager.h"

namespace Spire {

class Hub;
class StuObject;
class ShaderProgramAsset;
class VBOObject;
class IBOObject;

/// \todo For other pipes. If we have access to OpenGL4.2 features, use image
///       load store to implement Order Independent Transparency using
///       per-pixel linked lists. Requires atomic operations in the GPU.
///       Compositing will become very important in this renderer. We can only
///       guarantee a number of fragments in the shader.

/// \todo Before implementing an OIT renderer, a compositing renderer needs to
///       be built first. We can sort objects in a backt-to-front manner and
///       render them appropriately.

/// First pass, incredibly stupid pipe interface.
/// No frame buffer management or advanced rendering (although this kind of
/// advanced rendering would be more suited towards a programmable module that
/// you hook into spire). This pipe was only created for ease-of-use.
class StuInterface : public PipeInterface
{
public:
  StuInterface(Interface& iface);
  virtual ~StuInterface();
  
  /// Initialization as performed on the renderer thread.
  void ntsInitOnRenderThread() override;

  /// The different depths supported by the IBO.
  enum IBO_TYPE
  {
    IBO_8BIT,
    IBO_16BIT,
    IBO_32BIT,
  };

  /// Generally not needed at this stage.
  /// \todo Add supported OpenGL version to spire. This will allow us to 
  ///       determine what shaders we should us.
  /// \todo See glProvokingVertex for possible flat shading implementations
  ///       instead of using a geometry shader.
  /// \xxx  Place in GPUMan?
  enum SHADER_TYPES
  {
    // Programmable pipeline
    VERTEX_SHADER,          // Supported as of OpenGL 2.0
    TESSELATION_CONTROL,    // Supported as of OpenGL 4.0
    TESSELATION_EVALUATION, // Supported as of OpenGL 4.0
    GEOMETRY_SHADER,        // Supported as of OpenGL 3.2
    FRAGMENT_SHADER,        // Supported as of OpenGL 2.0

    // Abstract
    COMPUTE_SHADER,         // Supported as of OpenGL 4.3
  };

  /// See: http://www.opengl.org/wiki/Primitive
  /// \todo Add patch tesselation primitives.
  /// \xxx  Place in GPUMan?
  enum PRIMITIVE_TYPES
  {
    POINTS,
    LINES,
    LINE_LOOP,
    LINE_STRIP,
    TRIANGLES,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    LINES_ADJACENCY,
    LINE_STRIP_ADJACENCY,
    TRIANGLES_ADJACENCY,
    TRIANGLE_STRIP_ADJACENCY,
  };

  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================

  //---------
  // Objects
  //---------

  /// \todo Add BVH which only stores object names and is used for frustum
  ///       culling. This BVH should be mutex locked so the UI thread can cast
  ///       rays into it.

  /// Adds a renderable 'object' to the scene.
  void addObject(const std::string& object);

  /// Adds a renderable 'object' to the scene, and assigns it 'renderOrder'.
  /// Objects with lower render orders will be rendered first.
  void addObject(const std::string& object, int32_t renderOrder);

  /// Completely removes 'object' from the pipe. This includes removing all of
  /// the object's passes as well.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  void removeObject(const std::string& object);

  /// Removes all objects from the system.
  void removeAllObjects();

  /// Assigns a new rendering order to the object
  /// Throws std::range_error if object is not found.
  void assignRenderOrder(const std::string& object, int32_t renderOrder);


  /// Adds a VBO. This VBO can be re-used by adding passes to the object.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  /// \param  object        Name of the object to add the VBO to. The VBO will
  ///                       be destroyed when the object is removed. If you want
  ///                       to reuse VBO / IBOs add passes to this object.
  /// \param  name          Name of the VBO. See addIBOToObject for a full
  ///                       description of why you are required to name your
  ///                       VBO.
  /// \param  vboData       VBO data. This pointer will NOT be stored inside of
  ///                       spire. Unless there is a reference to it out side
  ///                       of spire, it will be destroyed.
  /// \param  attribNames   List of attribute names. This is used as a sanity
  ///                       check to ensure that the shader program's expected
  ///                       attributes match up with what you have provided in
  ///                       in the VBO. This only checked when a call to
  ///                       addPassToObject is made.
  void addVBO(const std::string& name,
              std::shared_ptr<std::vector<uint8_t>> vboData,
              const std::vector<std::string>& attribNames);

  /// Removes specified vbo from the object. It is safe to issue this call even
  /// though some of your passes may still be referencing the VBOs/IBOs. When
  /// the passes are destroyed, their associated VBOs/IBOs will be destroyed.
  void removeVBO(const std::string& vboName);

  /// Adds an IBO. This IBO can be re-used by adding passes to the object.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  /// \param  object        Name of the object to add the VBO to. The VBO will
  ///                       be destroyed when the object is removed. If you want
  ///                       to reuse VBO / IBOs add passes to this object.
  /// \param  name          Name of the IBO. You might find it odd that you are
  ///                       naming an IBO, and in certain terms you are right.
  ///                       IBOs are named here to avoid returning an identifier
  ///                       to the IBO. This would require a mutex lock (at
  ///                       the very least) *and* code would need to be run on
  ///                       the rendering thread since that is where the OpenGL
  ///                       context is current.
  /// \param  iboData       IBO data. This pointer will NOT be stored inside of
  ///                       spire. Unless there is a reference to it out side
  ///                       of spire, it will be destroyed.
  /// \param  type          Specifies what kind of IBO iboData represents.
  void addIBO(const std::string& name,
                      std::shared_ptr<std::vector<uint8_t>> iboData,
                      IBO_TYPE type);

  /// Removes specified ibo from the object. It is safe to issue this call even
  /// though some of your passes may still be referencing the VBOs/IBOs. When
  /// the passes are destroyed, their associated VBOs/IBOs will be destroyed.
  void removeIBO(const std::string& iboName);

  /// Adds a geometry pass to an object given by the identifier 'object'.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system. If there already exists a geometry pass, it throws a 'Duplicate' 
  /// exception.
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  /// \param  program       Complete shader program to use when rendering.
  ///                       See the oveloaded addPersistentShader functions.
  /// \param  vboID         VBO to use.
  /// \param  iboID         IBO to use.
  /// \return Pass ID. Use this ID to assign uniforms to the pass.
  void addPassToObject(const std::string& object,
                       const std::string& pass,
                       const std::string& program,
                       const std::string& vboName,
                       const std::string& iboName,
                       PRIMITIVE_TYPES type);
  void addPassToObject(const std::string& object,
                       const std::string& pass,
                       const std::string& program,
                       const std::string& vboName,
                       const std::string& iboName,
                       PRIMITIVE_TYPES type,
                       int32_t passOrder);

  /// Removes a geometry pass from the object.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system. 
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  void removeGeomPassFromObject(const std::string& object,
                                const std::string& pass);

  /// Associates a uniform value to the specified object's pass. If the uniform
  /// already exists, then its value will be updated if it passes a type check.
  /// During rendering the uniform value will be returned to its default value 
  /// once this pass has been completed.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system.
  /// Throws ShaderUniformTypeError if the types do not match what is stored
  /// in the shader.
  template <typename T>
  void addPassUniform(const std::string& object,
                      const std::string& pass,
                      const std::string& uniformName,
                      T uniformData)
  {
    addPassUniformConcrete(object, pass, uniformName, 
                           std::shared_ptr<AbstractUniformStateItem>(
                               new UniformStateItem<T>(uniformData)));
  }

  /// Concrete implementation of the above templated function.
  void addPassUniformConcrete(const std::string& object,
                              const std::string& pass,
                              const std::string& uniformName,
                              std::shared_ptr<AbstractUniformStateItem> item);

  /// GPU state that will be applied directly before the object is rendered.
  /// Note: The default GPU state is consists of the default GPUState 
  ///       constructor.
  void addPassGPUState(const std::string& object,
                       const std::string& pass,
                       const GPUState& state);

  /// Will add *or* update the global uniform if it already exsits.
  /// A shader of a given name is only allowed to be one type. If you attempt
  /// to bind different values to a uniform, this function will throw a
  /// ShaderUniformTypeError.
  template <typename T>
  void addGlobalUniform(const std::string& uniformName, T uniformData)
  {
    addGlobalUniformConcrete(uniformName, 
                             std::shared_ptr<AbstractUniformStateItem>(
                                 new UniformStateItem<T>(uniformData)));
  }

  /// Concrete implementation of the above templated function
  void addGlobalUniformConcrete(const std::string& uniformName,
                                std::shared_ptr<AbstractUniformStateItem> item);


  /// \todo Create method to add uniforms to the UniformManager (NOT the state
  ///       manager -- uniform manager is the type checker).

  //----------
  // Uniforms
  //----------
  
  
  //-----------------
  // Shader Programs
  //-----------------

  /// NOTE: It is possible to have shader programs implemented inside of the
  ///       pipe itself. That way, it's easier to detect OpenGL version problems
  ///       and other things.

  /// Adds a persistent shader under the name 'programName'. Default vertex and
  /// fragment shader.
  ///
  /// Throws an invalid_argument exception if there already exists a program
  /// by the same name, and the program specified by this function's arguments
  /// does not match the program that already exists.
  /// Throws a Duplicate exception if this shader is already in the persistent
  /// shader list.
  void addPersistentShader(const std::string& programName,
                           const std::string& vertexShader,
                           const std::string& fragmentShader);

  /// You can build any shader program you want using this method.
  /// Before you use this function, ensure that the necessary features are
  /// supported by the user's graphics card.
  ///
  /// Throws an invalid_argument exception if there already exists a program
  /// by the same name, and the program specified by this function's arguments
  /// does not match the program that already exists.
  /// Throws a Duplicate exception if this shader is already in the persistent
  /// shader list.
  ///
  /// \param shaders  First tuple argument is the shader program file, the 
  ///                 second is the type of shader.
  void addPersistentShader(const std::string& programName,
                           const std::vector<std::tuple<std::string, SHADER_TYPES>>& shaders);

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================
  // Be sure that you are calling these functions from the thread upon which
  // spire is executing. All non-thread-safe functions are prefixed with 'nts'.

  /// Perform the entire pass.
  /// \todo Add timing and other semantics here.
  void ntsDoPass() override;

  /// Obtain the current number of objects.
  size_t ntsGetNumObjects() const         {return mNameToObject.size();}

  /// Obtain the current rendering order.
  /// (technically, this is thread safe as the rendering thread never accesses
  ///  this value and is only used on the client thread).
  int32_t ntsGetRenderOrder() const       {return mCurrentRenderOrder;}

  /// Obtain the object associated with 'name'.
  /// throws std::range_error if the object is not found.
  std::shared_ptr<const StuObject> ntsGetObjectWithName(const std::string& name) const;
  
  /// Returns true if the system would render the list of object names in the
  /// specified order.
  bool ntsHasRenderingOrder(const std::vector<std::string>& renderOrder) const;

private:


  /// Remove the specified object from the order list.
  void removeObjectFromOrderList(const std::string& objectName, int32_t objectOrder);

  /// This unordered map is a 1-1 mapping of object names onto objects.
  std::unordered_map<std::string, std::shared_ptr<StuObject>>   mNameToObject;

  /// Rendering order of objects. This map is not a well-defined function: one
  /// value in the domain possibly maps to multiple values in the range.
  std::multimap<int32_t, std::shared_ptr<StuObject>>  mRenderOrderToObjects;

  /// List of shaders that are stored persistently by this pipe (will never
  /// be GC'ed unless this pipe is destroyed).
  std::list<std::shared_ptr<ShaderProgramAsset>>  mPersistentShaders;

  /// VBO names to our representation of a vertex buffer object.
  std::unordered_map<std::string, std::shared_ptr<VBOObject>>  mVBOMap;

  /// IBO names to our representation of an index buffer object.
  std::unordered_map<std::string, std::shared_ptr<IBOObject>>  mIBOMap;


  // NOTE:  The following variable should only be accessed on the client side.
  //        Never by the renderer. This var just makes it easier when adding
  //        objects and you don't care about their order.
  int32_t mCurrentRenderOrder;    ///< Current rendering order. Used for automatic order assignment.
  int32_t mCurrentPassOrder;      ///< Current pass rendering order.

private:

  /// Implementation functions executed on the renderer thread.
  /// I avoid references unless I know the objects that are being referenced
  /// will still be valid when execution reaches the renderer thread.
  /// (no stack variables allowed).
  /// @{
  static void addObjectImpl(Hub& hub, StuInterface* iface, std::string object,
                            int32_t renderOrder);

  static void assignRenderOrderImpl(Hub& hub, StuInterface* iface,
                                    std::string object, int32_t renderOrder);

  static void addIBOImpl(Hub& hub, StuInterface* iface,
                                 std::string iboName,
                                 std::shared_ptr<std::vector<uint8_t>> iboData,
                                 StuInterface::IBO_TYPE type);
  static void removeIBOImpl(Hub& hub, StuInterface* iface,
                                      std::string iboName);

  static void addVBOImpl(Hub& hub, StuInterface* iface,
                                 std::string vboName,
                                 std::shared_ptr<std::vector<uint8_t>> vboData,
                                 std::vector<std::string> attribNames);
  static void removeVBOImpl(Hub& hub, StuInterface* iface,
                                      std::string vboName);

  static void addPassToObjectImpl(Hub& hub, StuInterface* iface,
                                  std::string objectName,
                                  std::string passName,
                                  std::string program,
                                  std::string vboID,
                                  std::string iboID,
                                  PRIMITIVE_TYPES type,
                                  int32_t passOrder);

  static void addPassUniformInternalImpl(Hub& hub, StuInterface* iface,
                                         std::string object,
                                         std::string pass,
                                         std::string uniformName,
                                         std::shared_ptr<AbstractUniformStateItem> item);

  static void addPassGPUStateImpl(Hub& hub, StuInterface* iface,
                                  std::string object,
                                  std::string pass,
                                  GPUState state);

  static void addGlobalUniformInternalImpl(Hub& hub, StuInterface* iface,
                                           std::string uniformName,
                                           std::shared_ptr<AbstractUniformStateItem> item);

  static void removeGeomPassFromObjectImpl(Hub& hub, StuInterface* iface,
                                           std::string object,
                                           std::string pass);

  static void removeObjectImpl(Hub& hub, StuInterface* iface,
                               std::string object);
  static void removeAllObjectsImpl(Hub& hub, StuInterface* iface);

  static void addPersistentShaderImpl(Hub& hub, StuInterface* iface,
                                      std::string programName,
                                      std::vector<std::tuple<std::string, SHADER_TYPES>> shaders);
  /// @}
};

} // namespace Spire

#endif 
