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

#include "include/gaussian_rasterizer.h"

torch::Tensor GaussianRasterizer::markVisibleGaussians(
    torch::Tensor& positions) {
  // Mark visible points (based on frustum culling for camera) with a boolean
  torch::NoGradGuard no_grad;
  auto raster_settings = this->raster_settings_;
  return markVisible(positions, raster_settings.viewmatrix_,
                     raster_settings.projmatrix_);
}

torch::autograd::tensor_list GaussianRasterizerFunction::forward(
    torch::autograd::AutogradContext* ctx,
    torch::Tensor means3D,
    torch::Tensor means2D,
    torch::Tensor dc,
    torch::Tensor sh,
    torch::Tensor colors_precomp,
    torch::Tensor opacities,
    torch::Tensor scales,
    torch::Tensor rotations,
    torch::Tensor cov3Ds_precomp,
    GaussianRasterizationSettings raster_settings) {
  // Invoke C++/CUDA rasterizer
  auto [num_rendered, num_buckets, color, radii, geomBuffer, binningBuffer,
        imgBuffer, sampleBuffer] =
      RasterizeGaussiansCUDA(
          raster_settings.bg_, means3D, colors_precomp, opacities, scales,
          rotations, raster_settings.scale_modifier_, cov3Ds_precomp,
          raster_settings.viewmatrix_, raster_settings.projmatrix_,
          raster_settings.tanfovx_, raster_settings.tanfovy_,
          raster_settings.image_height_, raster_settings.image_width_, dc, sh,
          raster_settings.sh_degree_, raster_settings.campos_,
          raster_settings.prefiltered_, raster_settings.debug_);

  // Keep relevant tensors for backward
  ctx->saved_data["num_rendered"] = num_rendered;
  ctx->saved_data["num_buckets"] = num_buckets;
  ctx->saved_data["scale_modifier"] = raster_settings.scale_modifier_;
  ctx->saved_data["tanfovx"] = raster_settings.tanfovx_;
  ctx->saved_data["tanfovy"] = raster_settings.tanfovy_;
  ctx->saved_data["sh_degree"] = raster_settings.sh_degree_;
  ctx->saved_data["debug"] = raster_settings.debug_;
  ctx->save_for_backward({raster_settings.bg_, raster_settings.viewmatrix_,
                          raster_settings.projmatrix_, raster_settings.campos_,
                          colors_precomp, means3D, scales, rotations,
                          cov3Ds_precomp, radii, dc, sh, geomBuffer,
                          binningBuffer, imgBuffer, sampleBuffer});

  return {color, radii};
}

torch::autograd::tensor_list GaussianRasterizerFunction::backward(
    torch::autograd::AutogradContext* ctx,
    torch::autograd::tensor_list grad_outputs) {
  // Restore necessary values from context
  auto num_rendered = ctx->saved_data["num_rendered"].toInt();
  auto num_buckets = ctx->saved_data["num_buckets"].toInt();
  auto scale_modifier =
      static_cast<float>(ctx->saved_data["scale_modifier"].toDouble());
  auto tanfovx = static_cast<float>(ctx->saved_data["tanfovx"].toDouble());
  auto tanfovy = static_cast<float>(ctx->saved_data["tanfovy"].toDouble());
  auto sh_degree = ctx->saved_data["sh_degree"].toInt();
  auto debug = ctx->saved_data["debug"].toBool();

  auto saved = ctx->get_saved_variables();

  auto bg = saved[0];
  auto viewmatrix = saved[1];
  auto projmatrix = saved[2];
  auto campos = saved[3];
  auto colors_precomp = saved[4];
  auto means3D = saved[5];
  auto scales = saved[6];
  auto rotations = saved[7];
  auto cov3Ds_precomp = saved[8];
  auto radii = saved[9];
  auto dc = saved[10];
  auto sh = saved[11];
  auto geomBuffer = saved[12];
  auto binningBuffer = saved[13];
  auto imgBuffer = saved[14];
  auto sampleBuffer = saved[15];

  // Compute gradients for relevant tensors by invoking backward method
  auto grad_out_color = grad_outputs[0];
  auto [grad_means2D, grad_colors_precomp, grad_opacities, grad_means3D,
        grad_cov3Ds_precomp, grad_dc, grad_sh, grad_scales, grad_rotations] =
      RasterizeGaussiansBackwardCUDA(
          bg, means3D, radii, colors_precomp, scales, rotations, scale_modifier,
          cov3Ds_precomp, viewmatrix, projmatrix, tanfovx, tanfovy,
          grad_out_color, dc, sh, sh_degree, campos, geomBuffer, num_rendered,
          binningBuffer, imgBuffer, num_buckets, sampleBuffer, debug);

  return {
      grad_means3D,   grad_means2D,        grad_dc,
      grad_sh,        grad_colors_precomp, grad_opacities,
      grad_scales,    grad_rotations,      grad_cov3Ds_precomp,
      torch::Tensor()  //,
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor(),
                       // torch::Tensor()
  };
}

std::tuple<torch::Tensor, torch::Tensor> GaussianRasterizer::forward(
    torch::Tensor means3D,
    torch::Tensor means2D,
    torch::Tensor opacities,
    torch::Tensor dc,
    torch::Tensor shs,
    torch::Tensor colors_precomp,
    torch::Tensor scales,
    torch::Tensor rotations,
    torch::Tensor cov3D_precomp) {
  auto raster_settings = this->raster_settings_;

  if ((!shs.defined() /*shs is None*/ &&
       !colors_precomp.defined() /*colors_precomp is None*/) ||
      (shs.defined() /*shs is not None*/ &&
       colors_precomp.defined() /*colors_precomp is not None*/))
    throw std::runtime_error(
        "Please provide excatly one of either SHs or precomputed colors!");

  if (((!scales.defined() /*scales is None*/ ||
        !rotations.defined() /*rotations is None*/) &&
       !cov3D_precomp.defined() /*cov3D_precomp is None*/) ||
      ((scales.defined() /*scales is not None*/ ||
        rotations.defined() /*rotations is not None*/) &&
       cov3D_precomp.defined() /*cov3D_precomp is not None*/))
    throw std::runtime_error(
        "Please provide exactly one of either scale/rotation pair or "
        "precomputed 3D covariance!");

  torch::TensorOptions options;
  if (!shs.defined()) shs = torch::tensor({}, options.device(torch::kCUDA));
  if (!colors_precomp.defined())
    colors_precomp = torch::tensor({}, options.device(torch::kCUDA));
  if (!scales.defined())
    scales = torch::tensor({}, options.device(torch::kCUDA));
  if (!rotations.defined())
    rotations = torch::tensor({}, options.device(torch::kCUDA));
  if (!cov3D_precomp.defined())
    cov3D_precomp = torch::tensor({}, options.device(torch::kCUDA));

  auto result =
      rasterizeGaussians(means3D, means2D, dc, shs, colors_precomp, opacities,
                         scales, rotations, cov3D_precomp, raster_settings);

  return std::make_tuple(result[0] /*color*/, result[1] /*radii*/);
}

void SparseGaussianAdam::step(torch::Tensor& visibility, const uint32_t N) {
  torch::NoGradGuard no_grad;
  for (auto& group : this->param_groups()) {
    auto options = static_cast<torch::optim::AdamOptions&>(group.options());
    auto lr = options.lr();
    auto eps = options.eps();

    auto& param = group.params()[0];

    if (!param.grad().defined()) continue;

    auto& state = static_cast<torch::optim::AdamParamState&>(
        *this->state()[param.unsafeGetTensorImpl()]);
    if (!state.exp_avg().defined()) {
      state.step(0);
      state.exp_avg(
          torch::zeros_like(param, {}, torch::MemoryFormat::Preserve));
      state.exp_avg_sq(
          torch::zeros_like(param, {}, torch::MemoryFormat::Preserve));
    }

    auto exp_avg = state.exp_avg();
    auto exp_avg_sq = state.exp_avg_sq();
    auto grad = param.grad();

    const uint32_t M = param.numel() / N;

    adamUpdate(param, grad, exp_avg, exp_avg_sq, visibility, lr,
               std::get<0>(options.betas()), std::get<1>(options.betas()), eps,
               N, M);
  }
}
