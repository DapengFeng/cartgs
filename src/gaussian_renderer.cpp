/*
 * Copyright (C) 2023, Inria
 * GRAPHDECO research group, https://team.inria.fr/graphdeco
 * All rights reserved.
 *
 * This software is free for non-commercial, research and evaluation use
 * under the terms of the LICENSE.md file.
 *
 * For inquiries contact  george.drettakis@inria.fr
 *
 * This file is Derivative Works of Gaussian Splatting,
 * created by Longwei Li, Huajian Huang, Hui Cheng and Sai-Kit Yeung in 2023,
 * as part of Photo-SLAM and modified by Dapeng Feng in 2024, as part of CaRtGS.
 */

#include "include/gaussian_renderer.h"

/**
 * @brief
 *
 * @return std::tuple<render, viewspace_points, visibility_filter, radii>, which
 * are all `torch::Tensor`
 */
std::tuple<torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
GaussianRenderer::render(std::shared_ptr<GaussianKeyframe> viewpoint_camera,
                         int image_height,
                         int image_width,
                         std::shared_ptr<GaussianModel> pc,
                         GaussianPipelineParams& pipe,
                         torch::Tensor& bg_color,
                         torch::Tensor& override_color,
                         float scaling_modifier,
                         bool use_override_color) {
  /* Render the scene.

     Background tensor (bg_color) must be on GPU!
   */

  // Create zero tensor. We will use it to make pytorch return gradients of the
  // 2D (screen-space) means
  auto screenspace_points =
      torch::zeros_like(pc->getXYZ(), torch::TensorOptions()
                                          .dtype(pc->getXYZ().dtype())
                                          .requires_grad(true)
                                          .device(torch::kCUDA));
  try {
    screenspace_points.retain_grad();
  } catch (const std::exception& e) {
    // pass
  }

  // Set up rasterization configuration
  float tanfovx = std::tan(viewpoint_camera->FoVx_ * 0.5f);
  float tanfovy = std::tan(viewpoint_camera->FoVy_ * 0.5f);

  GaussianRasterizationSettings raster_settings(
      image_height, image_width, tanfovx, tanfovy, bg_color, scaling_modifier,
      viewpoint_camera->world_view_transform_,
      viewpoint_camera->full_proj_transform_, pc->active_sh_degree_,
      viewpoint_camera->camera_center_, false, false);

  GaussianRasterizer rasterizer(raster_settings);

  auto means3D = pc->getXYZ();
  auto means2D = screenspace_points;
  auto opacity = pc->getOpacityActivation();

  /* If precomputed 3d covariance is provided, use it. If not, then it will be
     computed from scaling / rotation by the rasterizer.
   */
  torch::Tensor scales, rotations, cov3D_precomp;
  if (pipe.compute_cov3D_) {
    cov3D_precomp = pc->getCovarianceActivation();
  } else {
    scales = pc->getScalingActivation();
    rotations = pc->getRotationActivation();
  }

  /* If precomputed colors are provided, use them. Otherwise, if it is desired
     to precompute colors from SHs in Python, do it. If not, then SH -> RGB
     conversion will be done by rasterizer.
   */
  torch::Tensor dc, shs, colors_precomp;
  if (use_override_color) {
    colors_precomp = override_color;
  } else {
    if (pipe.convert_SHs_) {
      int max_sh_degree = pc->max_sh_degree_ + 1;
      torch::Tensor shs_view = pc->getFeatures().transpose(1, 2).view(
          {-1, 3, max_sh_degree * max_sh_degree});
      torch::Tensor dir_pp =
          (pc->getXYZ() - viewpoint_camera->camera_center_.repeat(
                              {pc->getFeatures().size(0), 1}));
      auto dir_pp_normalized =
          dir_pp / torch::frobenius_norm(dir_pp, /*dim=*/{1}, /*keepdim=*/true);
      auto sh2rgb =
          sh_utils::eval_sh(pc->active_sh_degree_, shs_view, dir_pp_normalized);
      colors_precomp = torch::clamp_min(sh2rgb + 0.5, 0.0);
    } else {
      if (pipe.separate_sh_) {
        dc = pc->features_dc_.clone();
        shs = pc->features_rest_.clone();
      } else {
        shs = pc->getFeatures();
      }
    }
  }

  // Rasterize visible Gaussians to image, obtain their radii (on screen).
  auto rasterizer_result =
      rasterizer.forward(means3D, means2D, opacity, dc, shs, colors_precomp,
                         scales, rotations, cov3D_precomp);
  auto rendered_image = std::get<0>(rasterizer_result);
  auto radii = std::get<1>(rasterizer_result);

  /* Those Gaussians that were frustum culled or had a radius of 0 were not
     visible. They will be excluded from value updates used in the splitting
     criteria.
   */
  return std::make_tuple(
      rendered_image,                      /*render*/
      screenspace_points,                  /*viewspace_points*/
      (radii > 0).nonzero().reshape({-1}), /*visibility_filter*/
      radii /*radii*/);
}
