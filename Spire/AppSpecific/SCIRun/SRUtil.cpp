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
/// \date   August 2013

#include "SRUtil.h"

#include "Core/Common.h"

namespace Spire {
namespace SCIRun {

size_t buildNormalRenderingForVBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                                  size_t stride, float normalLength,
                                  std::vector<uint8_t>& out_vboData,
                                  std::vector<uint8_t>& out_iboData,
                                  size_t posOffset, size_t normOffset)
{
  uint8_t* rawData = &(*vboData)[0];

  // Figure out the size of out_vboData and out_iboData.
  int numInVertices = vboData->size() / stride;
  size_t outVBOSize = numInVertices * (sizeof(float) * 3 + sizeof(float) * 3) * 2;
  size_t outIBOSize = numInVertices * (sizeof(uint16_t) * 2);

  out_vboData->resize(outVBOSize);
  out_iboData->resize(outIBOSize);

  uint8_t* rawOutIBO = &out_iboData[0];
  uint8_t* rawOutVBO = &out_vboData[0];

  auto readFloat = [&rawData](size_t* offset) {
    float ret = *reinterpret_cast<float*>(&rawData[*offset]);
    *offset += sizeof(float);
    return ret;
  };

  auto writeFloat = [](uint8_t* rawData, size_t* offset, float data) {
    float* data = reinterpret_cast<float*>(&rawData[*offset]);
    *data = data;
  }

  auto writeUInt16 = [](uint8_t* rawData, size_t* offset, uint16_t data) {
    uint16_t* data = reinterpret_cast<uint16_t*>(&rawData[*offset]);
    *data = data;
  }

  size_t outVboPos = 0;
  size_t outIboPos = 0;
  uint16_t iboIndex = 0;
  for (size_t inputVboPos = 0; inputVboPos + stride < vboData->size();)
  {
    size_t inputOffset = inputVboPos + posOffset;
    V3 position;
    position.x = readFloat(&inputOffset);
    position.y = readFloat(&inputOffset);
    position.z = readFloat(&inputOffset);

    inputOffset = vboPos + normOffset;
    V3 normal;
    normal.x = readFloat(&inputOffset);
    normal.y = readFloat(&inputOffset);
    normal.z = readFloat(&inputOffset);

    inputVboPos += stride;

    V3 distal = position + normal * normalLength;

    // Write VBO
    writeFloat(rawOutVBO, &outVboPos, position.x);
    writeFloat(rawOutVBO, &outVboPos, position.y);
    writeFloat(rawOutVBO, &outVboPos, position.z);

    writeFloat(rawOutVBO, &outVboPos, distal.x);
    writeFloat(rawOutVBO, &outVboPos, distal.y);
    writeFloat(rawOutVBO, &outVboPos, distal.z);

    // Write ibo (all unique).
    writeUInt16(rawOutIBO, &outIboPos, iboIndex); ++iboIndex;
    writeUInt16(rawOutIBO, &outIboPos, iboIndex); ++iboIndex;
  }
}

} // namespace SCIRun
} // namespace Spire

