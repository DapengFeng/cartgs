// MIT License

// Copyright (c) 2024 {Mallick and Goel} and Kerbl, Bernhard and Vicente
// Carrasco, Francisco and Steinberger, Markus and De La Torre, Fernando

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// The Software constitues modifications to the 3D Gaussian Splatting codebase,
// which is licensed according to the text in "LICENSE_ORIGINAL". ONLY
// modifications made by the authors are licensed under the MIT License. To
// facilitate the identification of the modifications licensed under the MIT
// License, a diff file ("changes") highlighting these modifications is
// included.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <cuda.h>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#define GLM_FORCE_CUDA
#include <glm/glm.hpp>

namespace ADAM {

void adamUpdate(float* param,
                const float* param_grad,
                float* exp_avg,
                float* exp_avg_sq,
                const bool* tiles_touched,
                const float lr,
                const float b1,
                const float b2,
                const float eps,
                const uint32_t N,
                const uint32_t M);
}  // namespace ADAM
