/*
 * Copyright (C) 2023, Inria
 * GRAPHDECO research group, https://team.inria.fr/graphdeco
 * All rights reserved.
 *
 * This software is free for non-commercial, research and evaluation use
 * under the terms of the LICENSE.md file.
 *
 * For inquiries contact  george.drettakis@inria.fr
 */

#pragma once

#include <functional>
#include <tuple>
#include <vector>

namespace CudaRasterizer {
class Rasterizer {
 public:
  static void markVisible(int P,
                          float* means3D,
                          float* viewmatrix,
                          float* projmatrix,
                          bool* present);

  static std::tuple<int, int> forward(
      std::function<char*(size_t)> geometryBuffer,
      std::function<char*(size_t)> binningBuffer,
      std::function<char*(size_t)> imageBuffer,
      std::function<char*(size_t)> sampleBuffer,
      const int P,
      int D,
      int M,
      const float* background,
      const int width,
      int height,
      const float* means3D,
      const float* dc,
      const float* shs,
      const float* colors_precomp,
      const float* opacities,
      const float* scales,
      const float scale_modifier,
      const float* rotations,
      const float* cov3D_precomp,
      const float* viewmatrix,
      const float* projmatrix,
      const float* cam_pos,
      const float tan_fovx,
      float tan_fovy,
      const bool prefiltered,
      float* out_color,
      int* radii = nullptr,
      bool debug = false);

  static void backward(const int P,
                       int D,
                       int M,
                       int R,
                       int B,
                       const float* background,
                       const int width,
                       int height,
                       const float* means3D,
                       const float* dc,
                       const float* shs,
                       const float* colors_precomp,
                       const float* scales,
                       const float scale_modifier,
                       const float* rotations,
                       const float* cov3D_precomp,
                       const float* viewmatrix,
                       const float* projmatrix,
                       const float* campos,
                       const float tan_fovx,
                       float tan_fovy,
                       const int* radii,
                       char* geom_buffer,
                       char* binning_buffer,
                       char* image_buffer,
                       char* sample_buffer,
                       const float* dL_dpix,
                       float* dL_dmean2D,
                       float* dL_dconic,
                       float* dL_dopacity,
                       float* dL_dcolor,
                       float* dL_dmean3D,
                       float* dL_dcov3D,
                       float* dL_ddc,
                       float* dL_dsh,
                       float* dL_dscale,
                       float* dL_drot,
                       bool debug);
};
};  // namespace CudaRasterizer
