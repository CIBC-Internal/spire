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

#include <batch-testing/GlobalGTestEnv.hpp>
#include <batch-testing/SpireTestFixture.hpp>
#include "namespaces.h"

#include "spire/src/Common.h"
#include "spire/src/Exceptions.h"
#include "spire/src/SpireObject.h"
#include "spire/src/FileUtil.h"

#include "TestCommonUniforms.h"
#include "TestCommonAttributes.h"
#include "TestCamera.h"

using namespace spire;
using namespace CPM_BATCH_TESTING_NS;

namespace {

//------------------------------------------------------------------------------
TEST(SpireNoFixtureTests, TestSR5AssetLoader)
{
  std::ostringstream sRaw;

  // Write out the header.
  std::string header = "SCR5";
  sRaw.write(header.c_str(), 4);

  std::vector<V3> positions = 
  { V3(1.0f, 0.0f, 0.0f),
    V3(0.0f, 1.0f, 0.0f),
    V3(0.0f, 0.0f, 0.0f) };

  std::vector<V3> normals = 
  { V3(0.0f, 0.0f, 1.0f),
    V3(0.0f, 0.0f, 1.0f),
    V3(0.0f, 0.0f, 1.0f) };

  std::vector<uint16_t> indices = { 0,1,2 };

  // Helper functions for writing integers.
  auto writeUInt32 = [](std::ostream& ss, uint32_t i)
  { ss.write(reinterpret_cast<const char*>(&i), sizeof(uint32_t)); };

  auto writeUInt8 = [](std::ostream& ss, uint8_t i)
  { ss.write(reinterpret_cast<const char*>(&i), sizeof(uint8_t)); };

  // Number of meshes
  writeUInt32(sRaw, 1);

  // Number of vertices
  ASSERT_EQ(positions.size(), normals.size());
  writeUInt32(sRaw, static_cast<uint32_t>(positions.size()));

  // Write out the positions / normals.
  std::streamsize vec3Size = sizeof(float) * 3; // std::streamsize is the signed counterpart to size_t
  for (size_t i = 0; i < positions.size(); i++)
  {
    V3 pos = positions[i];
    V3 norm = normals[i];
    sRaw.write(reinterpret_cast<const char*>(glm::value_ptr(pos)), vec3Size);
    sRaw.write(reinterpret_cast<const char*>(glm::value_ptr(norm)), vec3Size);
  }

  // Ensure we have triangles.
  ASSERT_EQ(0, indices.size() % 3);

  // Number of faces
  writeUInt32(sRaw, static_cast<uint32_t>(indices.size() / 3));
  for (size_t i = 0; i < indices.size(); i+=3)
  {
    writeUInt8(sRaw, 3);
    sRaw.write(reinterpret_cast<const char*>(&indices[i+0]), sizeof(uint16_t));
    sRaw.write(reinterpret_cast<const char*>(&indices[i+1]), sizeof(uint16_t));
    sRaw.write(reinterpret_cast<const char*>(&indices[i+2]), sizeof(uint16_t));
  }

  // Now read the stringstream in and grab the resultant vectors.
  std::istringstream ss(sRaw.str());
  std::vector<uint8_t> vbo;
  std::vector<uint8_t> ibo;
  size_t numTriangles = Interface::loadProprietarySR5AssetFile(ss, vbo, ibo);

  ASSERT_EQ(1, numTriangles);

  //std::istreambuf_iterator<uint8_t>(&vbo[0]);
  // Construct strings from vbo and ibo (perfectly valid, the strings, when
  // given a size, can contain the null character).
  std::string vboStr(reinterpret_cast<char*>(&vbo[0]), vbo.size());
  std::string iboStr(reinterpret_cast<char*>(&ibo[0]), ibo.size());
  std::istringstream vboStream(vboStr);
  std::istringstream iboStream(iboStr);

  // Darn it! I want polymorphic lambdas!
  // The following 2 anonymous functions can be collapsed down to one with
  // polymorphic lambdas.
  auto verifySSFloat = [](float expectedVal, std::istream& iss)
  {
    float fromStream;
    iss.read(reinterpret_cast<char*>(&fromStream), sizeof(float));
    ASSERT_FLOAT_EQ(expectedVal, fromStream);
  };

  auto verifySSUInt16 = [](uint16_t expectedVal, std::istream& iss)
  {
    uint16_t fromStream;
    iss.read(reinterpret_cast<char*>(&fromStream), sizeof(uint16_t));
    ASSERT_EQ(expectedVal, fromStream);
  };

  auto checkVector = [&verifySSFloat](const V3& vec, std::istream& iss)
  {
    verifySSFloat(vec.x, iss);
    verifySSFloat(vec.y, iss);
    verifySSFloat(vec.z, iss);
  };

  for (size_t i = 0; i < positions.size(); i++)
  {
    checkVector(positions[i], vboStream);
    checkVector(normals[i], vboStream);
  }

  for (uint16_t i : indices)
    verifySSUInt16(i, iboStream);
}

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestPublicInterface)
{
  // This test is contrived and won't yield that much knowledge if you are 
  // attempting to learn the system.

  // REMEMBER:  We will always run the tests synchronously! So we will be able
  //            to catch errors immediately.

  std::string obj1 = "obj1";
  std::string obj2 = "obj2";
  std::string obj3 = "obj3";

  // We have a fresh instance of spire.
  mSpire->addObject(obj1);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_EQ(1, mSpire->getNumObjects());

  // Add a new obj2.
  mSpire->addObject(obj2);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj2), Duplicate);
  EXPECT_EQ(2, mSpire->getNumObjects());

  // Remove and re-add object 1.
  mSpire->removeObject(obj1);
  EXPECT_EQ(1, mSpire->getNumObjects());
  mSpire->addObject(obj1);
  EXPECT_EQ(2, mSpire->getNumObjects());

  // Add a new obj3.
  mSpire->addObject(obj3);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj2), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj3), Duplicate);
  EXPECT_EQ(3, mSpire->getNumObjects());
}

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestTriangle)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);
  // Test the rendering of a triangle.

  // Call Interface's doFrame manually. Then, since we are single threaded,
  // use the OpenGL context to extract a frame from the GPU and compare it with
  // golden images generated prior. If no golden image exists for this run,
  // then manually add it to the golden image comparison storage area...

  // First things first: just get the rendered image onto the filesystem...
  
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  0.0f,
     1.0f, -1.0f,  0.0f
  };
  std::vector<std::string> attribNames = {"aPos"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // This is pretty contorted interface due to the marshalling between
  // std::vector<float> and std::vector<uint8_t>. In practice, you would want
  // to calculate the size of your VBO using one std::vector<uint8_t> and
  // reserve the necessary space in it. Then cast it's contents to floats or
  // uint16_t as necessary (attributes can have a wide array of types, including
  // half floats).
  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Attempt to add duplicate VBOs and IBOs
  EXPECT_THROW(mSpire->addVBO(vbo1, rawVBO, attribNames), Duplicate);
  EXPECT_THROW(mSpire->addIBO(ibo1, rawIBO, iboType), Duplicate);

  std::string obj1 = "obj1";
  mSpire->addObject(obj1);
  
  std::string shader1 = "UniformColor";
  // Add and compile persistent shaders (if not already present).
  // You will only run into the 'Duplicate' exception if the persistent shader
  // is already in the persistent shader list.
  mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      });

  // Test various cases of shader failure after adding a prior shader.
  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::FRAGMENT_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::VERTEX_SHADER),
      }), std::invalid_argument);

  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor2.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      }), std::invalid_argument);

  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor2.fsh", Interface::FRAGMENT_SHADER),
      }), std::invalid_argument);

  // This final exception is throw directly from the addPersistentShader
  // function. The 3 prior exception were all thrown from the ShaderProgramMan.
  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      }), Duplicate);

  // Now construct passes (taking into account VBO attributes).

  // There exists no 'test obj'.
  EXPECT_THROW(mSpire->addPassToObject(
          "test obj", "UniformColor", "vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Not a valid shader.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "Bad Shader", "vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Non-existant vbo.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "UniformColor", "Bad vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Non-existant ibo.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "UniformColor", vbo1, "bad ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Build a good pass.
  std::string pass1 = "pass1";

  // Now add the object pass. This automatically adds the object to the pass for
  // us. But the ordering within the pass is still arbitrary.
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1);

  // Attempt to re-add the good pass.
  EXPECT_THROW(mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1),
               Duplicate);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);
  EXPECT_THROW(mSpire->removeIBO(ibo1), std::out_of_range);
  EXPECT_THROW(mSpire->removeVBO(vbo1), std::out_of_range);

  // Test global uniforms -- test run-time type validation.
  // Setup camera so that it can be passed to the Uniform Color shader.
  // Camera has been setup in the test fixture.
  mSpire->addGlobalUniform("uProjIVObject", myCamera->getWorldToProjection());
  EXPECT_THROW(mSpire->addGlobalUniform("uProjIVObject", V3(0.0f, 0.0f, 0.0f)), ShaderUniformTypeError);

  // Add color to the pass (which will lookup the type via the shader).
  EXPECT_THROW(mSpire->addObjectPassUniform(obj1, "uColor", V3(0.0f, 0.0f, 0.0f), pass1), ShaderUniformTypeError);
  EXPECT_THROW(mSpire->addObjectPassUniform(obj1, "uColor", M44(), pass1), ShaderUniformTypeError);
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f), pass1);

  beginFrame();
  mSpire->renderObject(obj1, pass1);

  compareFBOWithExistingFile(
      "stuTriangle.png",
      TEST_IMAGE_OUTPUT_DIR,
      TEST_IMAGE_COMPARE_DIR,
      TEST_PERCEPTUAL_COMPARE_BINARY,
      50);

  // Attempt to set global uniform value that is at odds with information found
  // in the uniform manager (should induce a type error).

  /// \todo Test adding a uniform to the global state which does not have a
  ///       corresponding entry in the UniformManager.

  /// \todo Test uniforms.
  ///       1 - No uniforms set: should attempt to access global uniform state
  ///           manager and extract the uniform resulting in a std::out_of_range.
  ///       2 - Partial uniforms. Result same as #1.
  ///       3 - Uniform type checking. Ensure the types pulled from OpenGL
  ///           compiler matches our expected types.


  // Create an image of appropriate dimensions.

  /// \todo Test pass order using hasPassRenderingOrder on the object.
}

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestObjectsStructure)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);

  // Test various functions in Object and ObjectPass.
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f,  0.0f,
     1.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  0.0f,
     1.0f, -1.0f,  0.0f
  };
  std::vector<std::string> attribNames = {"aPos"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // This is pretty contorted interface due to the marshalling between
  // std::vector<float> and std::vector<uint8_t>. In practice, you would want
  // to calculate the size of your VBO and using one std::vector<uint8_t> and
  // reserve the necessary space in it. Then cast it's contents to floats or
  // uint16_t as necessary (attributes can have a wide array of types, including
  // half floats).
  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]);
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]);
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Attempt to add duplicate VBOs and IBOs
  EXPECT_THROW(mSpire->addVBO(vbo1, rawVBO, attribNames), Duplicate);
  EXPECT_THROW(mSpire->addIBO(ibo1, rawIBO, iboType), Duplicate);

  std::string obj1 = "obj1";
  mSpire->addObject(obj1);
  
  std::string shader1 = "UniformColor";
  // Add and compile persistent shaders (if not already present).
  // You will only run into the 'Duplicate' exception if the persistent shader
  // is already in the persistent shader list.
  mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      });

  // Build the default pass.
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP);

  // Add a uniform *before* we add the next pass to ensure it gets properly
  // propogated to the new pass.
  mSpire->addObjectGlobalUniform(obj1, "uProjIVObject", myCamera->getWorldToProjection());

  // Construct another good pass.
  std::string pass1 = "pass1";
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);

  // Add pass uniforms for each pass.
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));    // default pass
  mSpire->addObjectGlobalUniform(obj1, "uColor", V4(1.0f, 0.0f, 1.0f, 1.0f));  // pass1

  //----------------------------------------------------------------------------
  // Test SpireObject structures
  //----------------------------------------------------------------------------
  std::shared_ptr<const SpireObject> object1 = mSpire->getObjectWithName(obj1);
  std::shared_ptr<const ObjectPass> object1Pass1 = object1->getObjectPassParams(pass1);
  std::shared_ptr<const ObjectPass> object1PassDefault = object1->getObjectPassParams(SPIRE_DEFAULT_PASS);

  EXPECT_EQ(2, object1->getNumPasses());
  EXPECT_EQ(true,  object1->hasGlobalUniform("uColor"));
  EXPECT_EQ(true,  object1->hasGlobalUniform("uProjIVObject"));
  EXPECT_EQ(false, object1->hasGlobalUniform("nonexistant"));

  EXPECT_EQ(false, object1Pass1->hasPassSpecificUniform("uColor"));
  EXPECT_EQ(true,  object1Pass1->hasUniform("uColor"));
  EXPECT_EQ(false, object1Pass1->hasPassSpecificUniform("uProjIVObject"));
  EXPECT_EQ(true,  object1Pass1->hasUniform("uProjIVObject"));

  EXPECT_EQ(true,  object1PassDefault->hasPassSpecificUniform("uColor"));
  EXPECT_EQ(true,  object1PassDefault->hasUniform("uColor"));
  EXPECT_EQ(false, object1PassDefault->hasPassSpecificUniform("uProjIVObject"));
  EXPECT_EQ(true,  object1PassDefault->hasUniform("uProjIVObject"));

  // Perform the frame. If there are any missing shaders we'll know about it
  // here.
  beginFrame();
  mSpire->renderObject(obj1);
}

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestRenderingWithSR5Object)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);
  // This test demonstrates a quick and dirty renderer using
  // attributes and lambdas. Spire knows nothing about the objects, but allows
  // sufficient flexibility that it is possible to do many things.

  // First things first: just get the rendered image onto the filesystem...
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::fstream sphereFile("Assets/UncappedCylinder.sp");
  Interface::loadProprietarySR5AssetFile(sphereFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // Add necessary VBO's and IBO's
  std::string vboName = "vbo1";
  std::string iboName = "ibo1";
  mSpire->addVBO(vboName, rawVBO, attribNames);
  mSpire->addIBO(iboName, rawIBO, iboType);

  // Build shaders
  std::string shaderName = "UniformColor";
  mSpire->addPersistentShader(
      shaderName, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      });

  // Add object
  std::string objectName = "obj1";
  mSpire->addObject(objectName);
  mSpire->addPassToObject(objectName, shaderName, vboName, iboName, 
                          Interface::TRIANGLE_STRIP);
  
  // Object pass uniforms (can be set at a global level)
  mSpire->addObjectPassUniform(objectName, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));    // default pass
  mSpire->addObjectGlobalUniform(objectName, "uProjIVObject", myCamera->getWorldToProjection());

//      M44 inverseViewProjection = iface.getGlobalUniform<M44>(
//          std::get<0>(TestCommonUniforms::getToCameraToProjection()));
//      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
//                                       it->shaderLocation, inverseViewProjection * objToWorld);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(iboName);
  mSpire->removeVBO(vboName);

  beginFrame();
  mSpire->renderObject(objectName);

  compareFBOWithExistingFile(
      "objectTest.png",
      TEST_IMAGE_OUTPUT_DIR,
      TEST_IMAGE_COMPARE_DIR,
      TEST_PERCEPTUAL_COMPARE_BINARY,
      50);
}


//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestRenderingWithAttributes)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);

  // This test demonstrates a quick and dirty renderer usin
  // attributes and lambdas. Spire knows nothing about the objects, but allows
  // sufficient flexibility that it is possible to do many things.

  // First things first: just get the rendered image onto the filesystem...
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::fstream sphereFile("Assets/Sphere.sp");
  Interface::loadProprietarySR5AssetFile(sphereFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // Add necessary VBO's and IBO's
  std::string vboName = "vbo1";
  std::string iboName = "ibo1";
  mSpire->addVBO(vboName, rawVBO, attribNames);
  mSpire->addIBO(iboName, rawIBO, iboType);

  // Build shaders
  std::string shaderName = "DirGouraud";
  mSpire->addPersistentShader(
      shaderName, 
      { std::make_tuple("DirGouraud.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("DirGouraud.fsh", Interface::FRAGMENT_SHADER),
      });

  // Add object
  std::string objectName = "obj1";
  mSpire->addObject(objectName);
  mSpire->addPassToObject(objectName, shaderName, vboName, iboName, 
                          Interface::TRIANGLE_STRIP);
  
  // Object pass uniforms (can be set at a global level)
  mSpire->addObjectPassUniform(objectName, "uAmbientColor", V4(0.1f, 0.1f, 0.1f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uDiffuseColor", V4(0.8f, 0.8f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uSpecularPower", 32.0f);

  // Object spire attributes (used for computing appropriate uniforms).
  M44 xform;
  xform[3] = V4(1.0f, 0.0f, 0.0f, 1.0f);
  mSpire->addObjectPassUniform(objectName, "uObject", xform);
  mSpire->addObjectGlobalUniform(objectName, "uProjIVObject",
                                 myCamera->getWorldToProjection() * xform);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(iboName);
  mSpire->removeVBO(vboName);

  // Global uniforms
  mSpire->addGlobalUniform("uLightDirWorld", V3(1.0f, 0.0f, 0.0f));

  // Setup camera uniforms.
  myCamera->setCommonUniforms(mSpire);

  beginFrame();
  mSpire->renderObject(objectName);

  compareFBOWithExistingFile(
      "attributeTest.png",
      TEST_IMAGE_OUTPUT_DIR,
      TEST_IMAGE_COMPARE_DIR,
      TEST_PERCEPTUAL_COMPARE_BINARY,
      50);
}

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestRenderingWithOutOfOrderAttributes)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);

  // Test the rendering of a phong shaded quad with out of order attributes.
  // aFieldData is unused in the phong shader.

  // Ensure shader is loaded.
  std::string shader1 = "DirPhong";
  mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("DirPhong.vsh", spire::Interface::VERTEX_SHADER), 
        std::make_tuple("DirPhong.fsh", spire::Interface::FRAGMENT_SHADER),
      });

  // Setup VBO / IBO.
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f
  };
  std::vector<std::string> attribNames = {"aPos", "aFieldData", "aNormal"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Setup object with default pass.
  std::string obj1 = "obj1";
  mSpire->addObject(obj1);
  
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP);
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);

  mSpire->addObjectPassUniform(obj1, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uSpecularColor", V4(0.0f, 0.0f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uSpecularPower", 16.0f);

  M44 xform;
  xform[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);
  mSpire->addObjectGlobalUniform(obj1, "uObject", xform);
  mSpire->addObjectGlobalUniform(obj1, "uProjIVObject", 
                                 myCamera->getWorldToProjection() * xform);

  // Setup uniforms unrelated to our object.
  mSpire->addGlobalUniform("uLightDirWorld", V3(0.0f, 0.0f, 1.0f));
  myCamera->setCommonUniforms(mSpire);

  beginFrame();
  mSpire->renderObject(obj1);

  compareFBOWithExistingFile(
      "orderOfAttributes.png",
      TEST_IMAGE_OUTPUT_DIR,
      TEST_IMAGE_COMPARE_DIR,
      TEST_PERCEPTUAL_COMPARE_BINARY,
      50);
}

}

