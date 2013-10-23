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

#ifndef SPIRE_INTERFACE_H
#define SPIRE_INTERFACE_H

#include <cstdint>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <memory>


#include "Context.h"

#include "Core/WinDLLExport.h"
#include "Core/Math.h"  // Necessary in order to communicate vector types.
#include "Core/ShaderUniformStateManTemplates.h"
#include "Core/GPUStateManager.h"

/// \todo The following *really* wants to be a constexpr inside of StuInterface,
/// when we upgrade to VS 2012, we should also upgrade this.
#define SPIRE_DEFAULT_PASS "spire_default"

#include "spire/cpm.h"
namespace CPM_NAMESPACE {
namespace Spire {

class Hub;
class HubThread;
class LambdaInterface;
class ObjectLambdaInterface;
class SpireObject;

/// Interface to the renderer.
/// A new interface will need to be created per-context.
/// Spire expects that only one thread will be communicating with it at any
/// given time.
class WIN_DLL Interface
{
public:
  /// All possible log outputs from the renderer. Used via the logging function.
  enum LOG_LEVEL
  {
    LOG_DEBUG,    ///< Debug / verbose.
    LOG_MESSAGE,  ///< General message.
    LOG_WARNING,  ///< Warning.
    LOG_ERROR,    ///< Error.
  };

  typedef std::function<void (const std::string&, Interface::LOG_LEVEL level)> 
      LogFunction;

  /// Constructs an interface to the renderer.
  /// \param  shaderDirs    A list of directories to search for shader files.
  /// \param  createThread  If true, then a thread will be created in which the
  ///                       renderer will run. The renderer will do it's best
  ///                       to manage time allocation and only use cycles
  ///                       when there is something to do.
  /// \param  logFP         The logging function to use when logging rendering
  ///                       messages. Remember, this function will be called
  ///                       from the rendering thread; so it will need to 
  ///                       communicate the messages in a thread-safe manner.
  /// @todo Re-enabled shared_ptr context when SCIRun5 adopts C++11.
  //Interface(std::shared_ptr<Context> context, bool createThread, 
  //          LogFunction logFP = LogFunction());
  Interface(std::shared_ptr<Context> context, 
            const std::vector<std::string>& shaderDirs,
            bool createThread, LogFunction logFP = LogFunction());
  virtual ~Interface();


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

  /// Used for setting up incoming shader attributes.
  enum DATA_TYPES
  {
    TYPE_BYTE,      ///< GLbyte   - 8-bit signed integer,   C-Type (signed char),   Suffix (b)
    TYPE_UBYTE,     ///< GLubyte  - 8-bit unsigned integer, C-Type (unsigned char), Suffix (ub)
    TYPE_SHORT,     ///< GLshort  - 16-bit integer,         C-Type (short),         Suffix (s)
    TYPE_USHORT,    ///< GLushort - 16-bit unsigned integer,C-Type (unsigned short),Suffix (us)
    TYPE_INT,       ///< GLint    - 32-bit integer          C-Type (long),          Suffix (I)
    TYPE_UINT,      ///< GLuint   - 32-bit unsigned integer,C-Type (unsigned long), Suffix (ui)
    TYPE_FLOAT,     ///< GLfloat  - 32-bit floating,        C-Type (float),         Suffix (f)
    TYPE_HALFFLOAT, ///< GLfloat  - 16-bit floating,        C-Type (?),             Suffix (?)
    TYPE_DOUBLE,    ///< GLdouble - 64-bit floating,        C-Type (double),        Suffix (d)
  };

  //============================================================================
  // CONCURRENT INTERFACE
  //============================================================================

  // Functions contained in the concurrent interface are not thread safe and
  // it is unlikely that they ever will be. In most scenarios, you should use
  // this concurrent interface instead of the threaded interface.

  struct UnsatisfiedUniform
  {
    UnsatisfiedUniform(const std::string& name, int location, unsigned int type) :
        uniformName(name),
        uniformType(type),
        shaderLocation(location)
    {}

    std::string                         uniformName;
    unsigned int                        uniformType;    // Should be: GLenum
    int                                 shaderLocation; // Should be: GLint
  };

  /// Callback issued when an object has unsatisfied uniforms. This is an
  /// oportunity for the function to remove items from the unsatisfied uniforms
  /// list. Once you have satisfied a particular uniform in the list, you
  /// should remove it. Otherwise, the rendering will bail on you.
  typedef std::function<void (std::list<UnsatisfiedUniform>&)> UnsatisfiedUniformCB;

  /// Calling this function is not necessary but may make your life.
  /// Here is a brief overview of what this function does:
  /// * Makes the context current if makeContextCurrent == true.
  /// * Checks the frame buffer status is complete (returns false if not).
  /// * Clears the depth buffer and color buffer (color buffer is cleared to 
  ///   [0,0,0,1]).
  /// * Sets a default GPU state by applying a fresh instance of the GPUState
  ///   class.
  /// You don't need to use this function. Just ensure that that the context
  /// which you passed into spire is current when you call into Spire.
  bool beginFrame(bool makeContextCurrent = true);

  /// This function just calls mContext->swapBuffers(). Nothing else.
  /// Call this as the complement to beginFrame when you are finished.
  /// Or don't call it and replace its functionality with something more
  /// appropriate for your needs.
  void endFrame();

  /// Renders an object given a specific pass.
  /// \todo Should we allow extra uniforms to be passed in, or should we stick
  ///       with the callback function for finding uniforms? Possibly add
  ///       the callback to this render call so it is clear what is happening
  ///       with regards to the world transformation. We had to set state with
  ///       lambdas when we were using the threaded interface, but we don't
  ///       need to do that here.
  ///       This would be the 'unsatisfied uniforms callback'.
  /// \todo Implement
  void renderObject(const std::string& objectName,
                    const UnsatisfiedUniformCB& cb = nullptr,
                    const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Adds a VBO. This VBO can be re-used by any objects in the system.
  /// \param  name          Name of the VBO. See addIBOToObject for a full
  ///                       description of why you are required to name your;t
  ///                       VBO.
  /// \param  vboData       VBO data. This pointer will NOT be stored in spire.
  /// \prama  vboSize       VBO data size.
  /// \param  attribNames   List of attribute names. This is used as a sanity
  ///                       check to ensure that the shader program's expected
  ///                       attributes match up with what you have provided in
  ///                       in the VBO. This only checked when a call to
  ///                       addPassToObject is made.
  void addVBO(const std::string& name,
              const uint8_t* vboData, size_t vboSize,
              const std::vector<std::string>& attribNames);

  /// Adds an IBO.
  /// \param  name          Name of the IBO.
  /// \param  iboData       IBO data. This pointer will NOT be stored in spire.
  ///                       No software backing of this pointer is made. The
  ///                       contents of this pointer are placed directly in an
  ///                       OpenGL buffer.
  /// \prama  iboSize       Size of iboData in bytes.
  /// \param  type          Specifies what kind of IBO iboData represents.
  void addIBO(const std::string& name, const uint8_t* iboData, size_t iboSize,
              IBO_TYPE type);

  /// Retrieve shader OpenGL ID's for custom rendering.

  /// Retrieve VBO and IBO ID's for custom rendering.

  /// Uniform retrieval.

  /// \todo Remove 'nts' from the names of the functions in the concurrent
  ///       interface.
  /// This call will render *only* the SPIRE_DEFAULT_PASS and nothing else.
  /// It is a barebones and minimal renderer.
  /// 
  /// You can call this function every time you want a new frame to be rendered
  /// unless you are using the threaded renderer. The threaded renderer will 
  /// call doFrame automatically.
  /// \note You must call doFrame on the same thread where makeCurrent was issued.
  ///       If this is not the same thread where Interface was created, ensure
  ///       a call to context->makeCurrent() is issued before invoking doFrame
  ///       for the first time.
  void ntsDoFrame();

  /// Obtain the current number of objects.
  /// \todo This function nedes to go to the implementation.
  size_t ntsGetNumObjects() const;

  /// Obtain the object associated with 'name'.
  /// throws std::range_error if the object is not found.
  std::shared_ptr<SpireObject> ntsGetObjectWithName(const std::string& name) const;

  /// Cleans up all GL resources.
  /// Should ONLY be called from the rendering thread.
  /// In our case, this amounts to disposing of all of our objects and VBO/IBOs
  /// and persistent shader objects.
  void ntsClearGLResources();

  /// Returns true if the specified object is in the pass.
  bool ntsIsObjectInPass(const std::string& object, const std::string& pass) const;

  /// Returns true if the pass already exists.
  bool ntsHasPass(const std::string& pass) const;

  //============================================================================
  // THREAD SAFE - Remember, the same thread should always be calling spire.
  //============================================================================

  // All thread safe functions can be called from the concurrent interface.
  // Additionally, when compiled without threading support, these functions
  // will be executed immediately 


  /// Terminates spire. If running 'threaded' then this will join with the 
  /// spire thread before returning. This should be called before the OpenGL
  /// context is destroyed.
  /// There is no mutex lock in this function, it should only be called by one
  /// thread.
  void terminate();

  //--------
  // Passes
  //--------

  // The default pass (SPIRE_DEFAULT_PASS) is always present.

  /// \todo Consider removal of manual passes from the system. There will be
  ///       no way of rendering these passes in the 'threaded' environment.

  /// Adds a pass to the front of the pass list. Passes at the front of the list
  /// are rendered first.
  void addPassToFront(const std::string& passName);

  /// Adds a pass to the back of the pass list. Passes at the back of the list
  /// are rendered last.
  void addPassToBack(const std::string& passName);

  //---------
  // Objects
  //---------

  /// Adds a renderable 'object' to the scene.
  void addObject(const std::string& object);

  /// Completely removes 'object' from the pipe. This includes removing all of
  /// the object's passes as well.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system.
  void removeObject(const std::string& object);

  /// Removes all objects from the system.
  void removeAllObjects();


  /// Same as addVBO in the concurrent interface, but using vectors and
  /// shared_ptrs to ensure the data survives its trip into the rendering
  /// thread.
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

  // Removes the specified vbo. It is safe to issue this call even though some
  // of your passes may still be referencing the VBOs/IBOs. When the passes are
  // destroyed, their associated VBOs/IBOs will be destroyed.
  void removeVBO(const std::string& vboName);

  /// Adds an IBO. Throws an std::out_of_range exception if the object is not
  /// found in the system.
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

  /// Loads an asset file and populates the given vectors with vbo and ibo
  /// data. In the future, we should expand this to include other asset types.
  /// Always uses 16bit IBOs and 32bit per component position / normal in the
  /// vbo.
  /// \return Returns the number of triangles read into ibo.
  /// \todo This should be moved into SCI-Run appspecific. It doesn't belong in
  ///       spire.
  static size_t loadProprietarySR5AssetFile(std::istream& stream,
                                            std::vector<uint8_t>& vbo,
                                            std::vector<uint8_t>& ibo);

  /// Adds a geometry stage to the front of the rendering order of the given
  /// pass on the object (mouthful!). If no other stage is added to the front
  /// of the object's pass, then this stage will be rendered before any other
  /// stage when this object is rendered.
  /// \param object         Name of the object to modify.
  /// \param program        The program to be executed during this substage.
  /// \param vboName        The VBO to use for rendering.
  /// \prama iboName        The IBO to use for rendering.
  /// \param type           The type of the primitives to be rendered.
  /// \param pass           Name of the pass to place this stage under.
  /// \param stage          Optional name for the stage. Can be left empty.
  /// \todo Implement
  void addObjectGeomPassToFront(const std::string& object,
                                const std::string& program,
                                const std::string& vboName,
                                const std::string& iboName,
                                PRIMITIVE_TYPES type,
                                const std::string& pass = SPIRE_DEFAULT_PASS,
                                const std::string& stage = "");

  /// Adds a geometry stage to the back of the rendering order of the given
  /// pass on the object (mouthful!). If no other stage is added to the back
  /// of the object's pass, then this stage will be rendered after every other
  /// stage when the object is rendered.
  /// \param object         Name of the object to modify.
  /// \param program        The program to be executed during this substage.
  /// \param vboName        The VBO to use for rendering.
  /// \prama iboName        The IBO to use for rendering.
  /// \param type           The type of the primitives to be rendered.
  /// \param pass           Name of the pass to place this stage under.
  /// \param stage          Optional name for the stage. Can be left empty.
  /// \todo Implement
  void addObjectGeomPassToBack(const std::string& object,
                               const std::string& program,
                               const std::string& vboName,
                               const std::string& iboName,
                               PRIMITIVE_TYPES type,
                               const std::string& pass = SPIRE_DEFAULT_PASS,
                               const std::string& stage = "");

  /// Adds a geometry pass to an object given by the identifier 'object'.
  /// Throws an std::out_of_range exception if the object is not found in the 
  /// system. If there already exists a geometry pass, it throws a 'Duplicate' 
  /// exception.
  /// \param  object        Unique object name.
  /// \param  program       Complete shader program to use when rendering.
  ///                       See the oveloaded addPersistentShader functions.
  /// \param  vboName       VBO to use.
  /// \param  iboName       IBO to use.
  /// \param  type          Primitive type.
  /// \param  pass          Pass name.
  /// \return Pass ID. Use this ID to assign uniforms to the pass.
  void addPassToObject(const std::string& object,
                       const std::string& program,
                       const std::string& vboName,
                       const std::string& iboName,
                       PRIMITIVE_TYPES type,
                       const std::string& pass = SPIRE_DEFAULT_PASS,
                       const std::string& parentPass = "");

  /// Removes a pass from the object.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system. 
  /// \param  object        Unique object name.
  /// \param  pass          Pass name.
  void removePassFromObject(const std::string& object,
                            const std::string& pass);


  //----------
  // Uniforms
  //----------
  
  /// Associates a uniform value to the specified object's pass. If the uniform
  /// already exists, then its value will be updated if it passes a type check.
  /// Throws an std::out_of_range exception if the object or pass is not found 
  /// in the system.
  /// Throws ShaderUniformTypeError if the types do not match what is stored
  /// in the shader.
  template <typename T>
  void addObjectPassUniform(const std::string& object,
                            const std::string& uniformName,
                            T uniformData,
                            const std::string& pass = SPIRE_DEFAULT_PASS)
  {
    addObjectPassUniformConcrete(object, uniformName, 
                                 std::shared_ptr<AbstractUniformStateItem>(
                                     new UniformStateItem<T>(uniformData)), pass);
  }

  /// Concrete implementation of the above templated function.
  void addObjectPassUniformConcrete(const std::string& object,
                                    const std::string& uniformName,
                                    std::shared_ptr<AbstractUniformStateItem> item,
                                    const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Adds a uniform that will be consumed regardless of the pass. Pass uniforms
  /// take precedence over pass global uniforms.
  template <typename T>
  void addObjectGlobalUniform(const std::string& object,
                              const std::string& uniformName,
                              T uniformData)
  {
    addObjectGlobalUniformConcrete(object, uniformName,
                                   std::shared_ptr<AbstractUniformStateItem>(
                                       new UniformStateItem<T>(uniformData)));
  }

  /// Concrete implementation of the above templated function.
  void addObjectGlobalUniformConcrete(const std::string& object,
                                      const std::string& uniformName,
                                      std::shared_ptr<AbstractUniformStateItem> item);

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

  /// GPU state that will be applied directly before the object is rendered.
  /// Note: The default GPU state is consists of the default GPUState 
  ///       constructor.
  void addObjectPassGPUState(const std::string& object,
                             const GPUState& state,
                             const std::string& pass = SPIRE_DEFAULT_PASS);

  //-------------------
  // Shader Attributes
  //-------------------

  // Attributes just as they are in the OpenGL rendering pipeline.
  void addShaderAttribute(const std::string& codeName, size_t numComponents,
                          bool normalize, size_t size, Interface::DATA_TYPES t);

  //----------------
  // Object Metadata
  //----------------
  template <typename T>
  void addObjectGlobalMetadata(const std::string& object,
                               const std::string& metadataName, T metadata)
  {
    addObjectGlobalMetadataConcrete(object, metadataName, 
                                    std::shared_ptr<AbstractUniformStateItem>(
                                        new UniformStateItem<T>(metadata)));
  }

  // Concrete implementation of the above templated function.
  void addObjectGlobalMetadataConcrete(const std::string& object,
                                       const std::string& metadataName,
                                       std::shared_ptr<AbstractUniformStateItem> item);

  template <typename T>
  void addObjectPassMetadata(const std::string& object,
                             const std::string& metadataName, T metadata,
                             const std::string& passName = SPIRE_DEFAULT_PASS)
  {
    addObjectPassMetadataConcrete(object, metadataName, 
                                  std::shared_ptr<AbstractUniformStateItem>(
                                      new UniformStateItem<T>(metadata)), passName);
  }

  // Concrete implementation of the above templated function.
  void addObjectPassMetadataConcrete(const std::string& object,
                                     const std::string& metadataName,
                                     std::shared_ptr<AbstractUniformStateItem> item,
                                     const std::string& passName = SPIRE_DEFAULT_PASS);

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

  //---------
  // Lambdas
  //---------

  // The complication in this lambda interface is due to threading.
  // If at all possible, use the concurrent interface.

  /// \note All lambdas use the push_back semantic. So if you are adding
  /// rendering lambdas, the first lambda you register will be the first one
  /// called when rendering.

  // Two types of lambdas to use. One with objects, and one with passes.
  // The name ObjectLambdaFunction is a little deceptive.
  // ObjectLambdaFunctions will be called per-pass.
  typedef std::function<void (LambdaInterface&)> PassLambdaFunction;

  // Lambda function that includes an object as context.
  typedef std::function<void (ObjectLambdaInterface&)> ObjectLambdaFunction;

  /// These functions help satisfy uniforms that need extra attribute data in
  /// order to process. These can be used to remove load from the GPU by
  /// precomputing any number of things.
  typedef std::function<void (ObjectLambdaInterface&, std::list<UnsatisfiedUniform>&)> 
      ObjectUniformLambdaFunction;

  /// \todo Remove these functions. They were necessary only for the threaded
  ///       version of spire.
  /// The following functions add hooks into the rendering infrastructure.
  /// @{
  void addLambdaBeginAllPasses(const PassLambdaFunction& fp);
  void addLambdaEndAllPasses(const PassLambdaFunction& fp);
  void addLambdaPrePass(const PassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  void addLambdaPostPass(const PassLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);
  /// @}
  
  /// Adds per-object hooks for calculating uniforms and performing rendering.
  /// Both are optional, and you can add as many lambdas as you need.
  /// @{

  /// If an object rendering lambda is found, then normal rendering does not
  /// proceed.
  void addLambdaObjectRender(const std::string& object, const ObjectLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// Lambda object uniforms are optional and they will not be called if there
  /// are no unsatisfied uniforms found.
  void addLambdaObjectUniforms(const std::string& object, const ObjectUniformLambdaFunction& fp, const std::string& pass = SPIRE_DEFAULT_PASS);

  /// @}

  //============================================================================
  // NOT THREAD SAFE
  //============================================================================


protected:

  std::unique_ptr<Hub>      mHub;   ///< Rendering hub.

};

} // namespace spire
} // namespace CPM_NAMESPACE

#endif // SPIRE_INTERFACE_H