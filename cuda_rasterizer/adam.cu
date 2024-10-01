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

#include <cooperative_groups.h>
#include <cooperative_groups/reduce.h>

#include "adam.h"
#include "auxiliary.h"
namespace cg = cooperative_groups;

// step on a grid of size (N, M)
// N is always number of gaussians
__global__ void adamUpdateCUDA(float* __restrict__ param,
                               const float* __restrict__ param_grad,
                               float* __restrict__ exp_avg,
                               float* __restrict__ exp_avg_sq,
                               const bool* tiles_touched,
                               const float lr,
                               const float b1,
                               const float b2,
                               const float eps,
                               const uint32_t N,
                               const uint32_t M) {
  auto p_idx = cg::this_grid().thread_rank();
  const uint32_t g_idx = p_idx / M;
  if (g_idx >= N) return;
  if (tiles_touched[g_idx]) {
    float Register_param_grad = param_grad[p_idx];
    float Register_exp_avg = exp_avg[p_idx];
    float Register_exp_avg_sq = exp_avg_sq[p_idx];
    Register_exp_avg =
        b1 * Register_exp_avg + (1.0f - b1) * Register_param_grad;
    Register_exp_avg_sq = b2 * Register_exp_avg_sq + (1.0f - b2) *
                                                         Register_param_grad *
                                                         Register_param_grad;
    float step = -lr * Register_exp_avg / (sqrt(Register_exp_avg_sq) + eps);

    param[p_idx] += step;
    exp_avg[p_idx] = Register_exp_avg;
    exp_avg_sq[p_idx] = Register_exp_avg_sq;
  }
}

void ADAM::adamUpdate(float* param,
                      const float* param_grad,
                      float* exp_avg,
                      float* exp_avg_sq,
                      const bool* tiles_touched,
                      const float lr,
                      const float b1,
                      const float b2,
                      const float eps,
                      const uint32_t N,
                      const uint32_t M) {
  const uint32_t cnt = N * M;
  adamUpdateCUDA<<<(cnt + 255) / 256, 256>>>(param, param_grad, exp_avg,
                                             exp_avg_sq, tiles_touched, lr, b1,
                                             b2, eps, N, M);
}
