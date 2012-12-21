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

#include "BaseAssetMan.h"
#include "MurmurHash3.h"

namespace Spire {

//------------------------------------------------------------------------------
BaseAssetMan::BaseAssetMan()
{
}

//------------------------------------------------------------------------------
BaseAssetMan::~BaseAssetMan()
{
}

//------------------------------------------------------------------------------
void BaseAssetMan::addAsset(std::shared_ptr<BaseAsset> asset)
{
  mAssets.push_back(std::weak_ptr<BaseAsset>(asset));
}

//------------------------------------------------------------------------------
void BaseAssetMan::holdAsset(std::shared_ptr<BaseAsset> asset, 
                             int64_t absTimeToHold)
{
  asset->setAbsTimeToHold(absTimeToHold);
  mHeldAssets.push(asset);
}

//------------------------------------------------------------------------------
void BaseAssetMan::updateOrphanedAssets(int64_t absTime)
{
  // Check the first held asset and if it is less than the absolute time,
  // release it and continue.
  while (mHeldAssets.empty() == false)
  {
    if (mHeldAssets.top()->getAbsTimeHeld() < absTime)
      mHeldAssets.pop();
    else 
      break;
  }

  // Iterate over all of the assets and remove expired elements.
  auto it = mAssets.begin();
  while (it != mAssets.end())
  {
    if (it->expired())
    {
      // Remove this element from the list since it has expired.
      it = mAssets.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//------------------------------------------------------------------------------
// Base Asset
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
BaseAsset::BaseAsset(const std::string& name) :
    mName(name),
    mAbsHoldTime(0)
{
  MurmurHash3_x86_32(
      static_cast<const void*>(name.c_str()), static_cast<int>(name.size()),
      mHashSeed, static_cast<void*>(&mNameHash));
}

//------------------------------------------------------------------------------
BaseAsset::~BaseAsset()
{
}

} // end of namespace Spire