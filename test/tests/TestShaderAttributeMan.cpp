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

#include <gtest/gtest.h>
#include "namespaces.h"
#include "spire/src/Common.h"
#include "spire/src/Exceptions.h"
#include "spire/src/ShaderAttributeMan.h"

using namespace spire;

namespace {

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestUnknownAttribute)
{
  ShaderAttributeMan attribMan;

  ASSERT_EQ(1, attribMan.getNumAttributes());

  std::string unknownName = ShaderAttributeMan::getUnknownName();
  AttribState state;

  // Test function return values.
  EXPECT_NO_THROW(state = attribMan.getAttributeAtIndex(
          ShaderAttributeMan::getUnknownAttributeIndex()));
  EXPECT_EQ(true, std::get<0>(attribMan.findAttributeWithName(unknownName)));
  EXPECT_EQ(0, std::get<1>(attribMan.findAttributeWithName(unknownName)));

  // Test returned attribMan.
  EXPECT_EQ(0, state.index);
}

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestUnknownExceptions)
{
  ShaderAttributeMan attribMan;

  // Attempt to access the element passed the end of the array.
  EXPECT_THROW(
      attribMan.getAttributeAtIndex(1),
      std::range_error);

  std::string bogusName = "someName";
  EXPECT_THROW(attribMan.getAttributeWithName(bogusName), NotFound);

  EXPECT_EQ(false, std::get<0>(attribMan.findAttributeWithName(bogusName)));
  EXPECT_EQ(0, std::get<1>(attribMan.findAttributeWithName(bogusName)));
}

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestDefaultAttributes)
{
  ShaderAttributeMan attribMan; // true

  EXPECT_LE(1, attribMan.getNumAttributes());
}

//------------------------------------------------------------------------------
class ShaderAttributeManInvolved : public testing::Test
{
protected:
  ShaderAttributeManInvolved() {}

  virtual void SetUp()    {addDefaultAttributes();}
  virtual void TearDown() {}

  void addDefaultAttributes()
  {
    mAttribMan.addAttribute("aPos",         3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aNormal",      3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aColor",       4,  true,   sizeof(char) * 4,   spire::Interface::TYPE_UBYTE);
  }

  ShaderAttributeMan  mAttribMan;
};

//------------------------------------------------------------------------------
TEST_F(ShaderAttributeManInvolved, addingAttributes)
{
  // When using a test fixture it is the same as deriving from the fixture class.
  size_t beginSize = mAttribMan.getNumAttributes();

  // Add attributes and ensure that they have been appropriately added.
  std::string attribName = "att1";
  mAttribMan.addAttribute(attribName, 3, false, 
                          sizeof(float) * 3, Interface::TYPE_FLOAT);

  // This test uses a different method of obtaining the attribute than the
  // next attribute. This uses findAttributeWithName, while the other uses
  // getAttributeWithName and expects that no exceptions are thrown.
  AttribState state;
  std::tuple<bool, size_t> attIndex = mAttribMan.findAttributeWithName(attribName);
  ASSERT_EQ(true, std::get<0>(attIndex));
  state = mAttribMan.getAttributeAtIndex(std::get<1>(attIndex));
  EXPECT_EQ(beginSize, state.index);
  EXPECT_EQ(attribName, state.codeName);
  EXPECT_EQ(mAttribMan.hashString(attribName), state.nameHash);
  EXPECT_EQ(3, state.numComponents);
  EXPECT_EQ(false, state.normalize);
  EXPECT_EQ(sizeof(float) * 3, state.size);
  EXPECT_EQ(Interface::TYPE_FLOAT, state.type);


  attribName = "att2";
  mAttribMan.addAttribute(attribName, 2, true, 
                          sizeof(char) * 2, Interface::TYPE_UBYTE);
  ASSERT_NO_THROW(state = mAttribMan.getAttributeWithName(attribName));
  EXPECT_EQ(beginSize + 1, state.index);
  EXPECT_EQ(attribName, state.codeName);
  EXPECT_EQ(mAttribMan.hashString(attribName), state.nameHash);
  EXPECT_EQ(2, state.numComponents);
  EXPECT_EQ(true, state.normalize);
  EXPECT_EQ(sizeof(char) * 2, state.size);
  EXPECT_EQ(Interface::TYPE_UBYTE, state.type);

  EXPECT_EQ(beginSize + 2, mAttribMan.getNumAttributes());
}

//------------------------------------------------------------------------------
class ShaderAttributeCollectionTests : public testing::Test
{
protected:
  ShaderAttributeCollectionTests() :
      mAttribMan(),
      mCol1(mAttribMan),
      mCol2(mAttribMan),
      mCol3(mAttribMan)
  {}

  virtual void SetUp()
  {
    addDefaultAttributes();

    mAttribMan.addAttribute("at1",  3,  false,  sizeof(float)*3,  Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("at2",  3,  false,  sizeof(float)*3,  Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("at3",  1,  false,  sizeof(float),    Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("at4",  4,  false,  sizeof(char)*4,   Interface::TYPE_UBYTE);

    mCol1.addAttribute("at1");
    mCol1.addAttribute("at3");

    /// The following are purposefully out-of-order
    mCol2.addAttribute("at1");
    mCol2.addAttribute("at3");
    mCol2.addAttribute("at2");

    mCol3.addAttribute("at4");
    mCol3.addAttribute("at1");
    mCol3.addAttribute("at3");
    mCol3.addAttribute("at2");
  }
  virtual void TearDown() {}

  void addDefaultAttributes()
  {
    mAttribMan.addAttribute("aPos",         3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aNormal",      3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  spire::Interface::TYPE_FLOAT);
    mAttribMan.addAttribute("aColor",       4,  true,   sizeof(char) * 4,   spire::Interface::TYPE_UBYTE);
  }

  ShaderAttributeMan          mAttribMan;
  ShaderAttributeCollection   mCol1;
  ShaderAttributeCollection   mCol2;
  ShaderAttributeCollection   mCol3;
};

//------------------------------------------------------------------------------
TEST_F(ShaderAttributeCollectionTests, primaryTest)
{
  AttribState state;

  // Check first attribute and most of its values.
  ASSERT_NO_THROW(state = mCol1.getAttribute(0));
  EXPECT_EQ("at1", state.codeName);
  EXPECT_EQ(3, state.numComponents);
  EXPECT_EQ(false, state.normalize);
  EXPECT_EQ(sizeof(float) * 3, state.size);
  EXPECT_EQ(Interface::TYPE_FLOAT, state.type);

  // Now just check that the attributes are in the expected order.
  ASSERT_NO_THROW(state = mCol1.getAttribute(1));
  EXPECT_EQ("at3", state.codeName);

  ASSERT_NO_THROW(state = mCol2.getAttribute(0));
  EXPECT_EQ("at1", state.codeName);
  ASSERT_NO_THROW(state = mCol2.getAttribute(1));
  EXPECT_EQ("at3", state.codeName);
  ASSERT_NO_THROW(state = mCol2.getAttribute(2));
  EXPECT_EQ("at2", state.codeName);

  ASSERT_NO_THROW(state = mCol3.getAttribute(0));
  EXPECT_EQ("at4", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getAttribute(1));
  EXPECT_EQ("at1", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getAttribute(2));
  EXPECT_EQ("at3", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getAttribute(3));
  EXPECT_EQ("at2", state.codeName);
}

}
